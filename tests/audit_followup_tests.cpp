// audit_followup_tests.cpp
//
// Follow-up audit for MSPA 1.1.2. Closes three weaknesses found in review of
// tests/audit_behavior_tests.cpp @ 9c0a8f740fdac51dba886a19e2c0456d1349d95e:
//
//   H*  Hysteresis must damp noise but must NOT lock the agent onto a worse option.
//   C*  "Stable agents calm faster" must be tested on decay SPEED, not on final level
//       (the old test also passes if only the tonic baseline differs).
//   P*  Personality must change DECISIONS, not just beliefs and emotions.
//
// All assertions are relative or property-based; no tuned magic numbers about outputs.
// A failure here is a finding about the library, not a reason to edit the test.
//
// NOTE: written against the public API exactly as used in audit_behavior_tests.cpp.
// It has not been compiled. Names to double-check on first build: CandidateScore::action
// and ::utility (used by check_trace in the existing file), Personality::boldness/stability.

#include "test.hpp"

#include "mspa/mspa.hpp"
#include "mspa/psychology/emotions.hpp"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

using namespace mspa;

namespace {

std::unique_ptr<Runtime> make_runtime(std::uint64_t rid) {
    RuntimeConfig c{};
    c.runtime_id = RuntimeId(rid);
    c.world_id = WorldId(1);
    c.time_domain = {TimeDomainId(1), {60, 1}};
    c.seed_root = 12345;
    auto rr = Runtime::create(c, RegistrySnapshot{});
    require(rr, "create runtime");
    return std::move(*rr.value_if());
}

// Deterministic noise source owned by the test (independent of MSPA's RNG).
struct Lcg {
    std::uint64_t s;
    float next() {  // [0,1)
        s = s * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<float>((s >> 40) & 0xFFFFFFu) / 16777216.0f;
    }
};

// ---------------------------------------------------------------------------
// H: hysteresis
// ---------------------------------------------------------------------------

std::vector<ActionOption> two_options(AgentId a, bool continuity, float cost1, float cost2) {
    ActionOption x{};
    x.type = ActionTypeId(1);
    x.instance = ActionInstanceId(1);
    x.actor = a;
    x.continuity = continuity ? ActionContinuityKey(1) : ActionContinuityKey{};
    x.cost = cost1;
    x.progress = 0.35f;
    x.interruption_cost = 0.30f;
    x.context_stability = 1.0f;
    ActionOption y = x;
    y.instance = ActionInstanceId(2);
    y.continuity = continuity ? ActionContinuityKey(2) : ActionContinuityKey{};
    y.cost = cost2;
    return {x, y};
}

std::uint64_t pick(Runtime& r, AgentId a, const std::vector<ActionOption>& o, std::uint64_t t) {
    auto d = r.decide(a, o, PsychTime(t));
    require(d && d.value_if()->has_selection, "decide must select an option");
    return d.value_if()->selected_action.value();
}

AgentId new_sticky_agent(Runtime& r, std::uint64_t local) {
    AgentId a{WorldId(1), local};
    AgentInit init{};
    init.personality.persistence = 0.85f;  // same profile as the original hysteresis test
    init.personality.stability = 0.85f;
    require(r.create_agent(a, init, PsychTime(0)), "create sticky agent");
    return a;
}

constexpr int kWarmup = 20;
constexpr int kPhase2 = 40;

struct SwitchResult {
    int latency;        // decisions until option 2 is chosen (kPhase2+1 = never)
    int changes_after;  // changes of selection after option 2 was first chosen
};

// Option 1 is slightly better during warmup; then option 2 becomes better by `gap`.
SwitchResult clear_improvement(float gap, bool continuity, std::uint64_t runtime_id) {
    auto r = make_runtime(runtime_id);
    AgentId a = new_sticky_agent(*r, 1);
    std::uint64_t t = 0, last = 0;
    for (int i = 0; i < kWarmup; ++i)
        last = pick(*r, a, two_options(a, continuity, 0.60f, 0.61f), ++t);

    SwitchResult res{kPhase2 + 1, 0};
    bool reached = false;
    for (int i = 1; i <= kPhase2; ++i) {
        const std::uint64_t now = pick(*r, a, two_options(a, continuity, 0.60f, 0.60f - gap), ++t);
        if (!reached && now == 2) {
            reached = true;
            res.latency = i;
        } else if (reached && now != last) {
            ++res.changes_after;
        }
        last = now;
    }
    return res;
}

void test_hysteresis_does_not_lock() {
    const float gaps[] = {0.02f, 0.05f, 0.10f, 0.20f, 0.35f, 0.50f};
    int prev_latency = kPhase2 + 1;
    int last_latency = kPhase2 + 1;
    for (int i = 0; i < 6; ++i) {
        const auto with = clear_improvement(gaps[i], true, 100 + 2 * i);
        const auto without = clear_improvement(gaps[i], false, 101 + 2 * i);

        // Positive control: without continuity the better option is taken immediately.
        require(without.latency == 1, "control: without continuity the better option must be chosen at once");

        // Larger improvements must never be adopted later than smaller ones.
        require(with.latency <= prev_latency, "a larger improvement must not switch later than a smaller one");

        // Once switched, no flip-flopping back.
        if (with.latency <= kPhase2)
            require(with.changes_after == 0, "after adopting the better option the agent must not flip back");

        prev_latency = with.latency;
        last_latency = with.latency;
    }
    require(last_latency <= 3, "a clearly better option (gap 0.50) must be adopted within 3 decisions");
}

int noisy_switches(bool continuity, std::uint64_t runtime_id) {
    auto r = make_runtime(runtime_id);
    AgentId a = new_sticky_agent(*r, 1);
    Lcg rng{987654321ULL};  // identical noise sequence for both variants
    std::uint64_t last = 0;
    int switches = 0;
    for (int i = 1; i <= 100; ++i) {
        const float c1 = 0.50f + 0.04f * (rng.next() - 0.5f);
        const float c2 = 0.50f + 0.04f * (rng.next() - 0.5f);
        const std::uint64_t now = pick(*r, a, two_options(a, continuity, c1, c2), static_cast<std::uint64_t>(i));
        if (last && now != last) ++switches;
        last = now;
    }
    return switches;
}

void test_hysteresis_under_random_noise() {
    const int with = noisy_switches(true, 200);
    const int without = noisy_switches(false, 201);
    require(with <= 10, "with continuity, random near-tie noise must cause few switches");
    require(without >= 30, "without continuity, random near-tie noise must cause many switches");
    require(without >= 3 * with, "hysteresis must cut switching at least threefold under random noise");
}

// ---------------------------------------------------------------------------
// C: decay speed (not just final level)
// ---------------------------------------------------------------------------

float fear_of(const AgentPsychState& a) { return EmotionSystem{}.intensity(a, BasicEmotion::Fear); }
float fear_now(Runtime& r, AgentId id) { return fear_of(*r.agents().find(id)); }

void make_fearful_agent(Runtime& r, AgentId id, const Personality& p, std::uint64_t at) {
    AgentInit init{};
    init.personality = p;
    init.emotions = {{BasicEmotion::Fear, 0.90f, PsychTime(at), PsychTime(at)}};
    require(r.create_agent(id, init, PsychTime(at)), "create fearful agent");
}

struct Curve {
    std::vector<std::uint64_t> ticks;
    std::vector<float> fear;
    float baseline = 0.0f;
};

Curve decay_curve(Runtime& r, AgentId id, const Personality& p) {
    make_fearful_agent(r, id, p, 0);
    Curve c;
    std::uint64_t last = 0;
    for (double t = 1.0; t < 2.0e6; t *= 1.4142135) {  // geometric grid, ~42 samples
        const auto tick = static_cast<std::uint64_t>(t);
        if (tick <= last) continue;
        require(r.advance_to(id, PsychTime(tick)), "advance along decay curve");
        c.ticks.push_back(tick);
        c.fear.push_back(fear_now(r, id));
        last = tick;
    }
    require(r.advance_to(id, PsychTime(100000000ULL)), "advance to baseline");
    c.baseline = fear_now(r, id);  // late value = this agent's tonic baseline
    return c;
}

void test_decay_speed() {
    auto r = make_runtime(300);
    Personality neutral{};
    Personality stable{};
    stable.stability = 0.95f;
    Personality unstable{};
    unstable.stability = 0.10f;

    const auto n1 = decay_curve(*r, {WorldId(1), 1}, neutral);
    const auto n2 = decay_curve(*r, {WorldId(1), 2}, neutral);
    const auto st = decay_curve(*r, {WorldId(1), 3}, stable);
    const auto un = decay_curve(*r, {WorldId(1), 4}, unstable);

    // C1: null control. Identical personalities must give identical curves.
    require(n1.fear.size() == n2.fear.size(), "same grid for control agents");
    for (std::size_t i = 0; i < n1.fear.size(); ++i)
        require(std::fabs(n1.fear[i] - n2.fear[i]) < 1.0e-5f, "identical personalities must decay identically");

    // Fear must never rise without new stimuli.
    for (const Curve* c : {&n1, &st, &un})
        for (std::size_t i = 1; i < c->fear.size(); ++i)
            require(c->fear[i] <= c->fear[i - 1] + 1.0e-6f, "fear must not increase without new stimuli");

    // C2: compare the REMAINING FRACTION of the excess over each agent's own baseline.
    require(0.90f - st.baseline > 0.1f && 0.90f - un.baseline > 0.1f, "episodic peak must be well above baseline");
    int compared = 0;
    for (std::size_t i = 0; i < st.fear.size() && i < un.fear.size(); ++i) {
        const float rs = (st.fear[i] - st.baseline) / (0.90f - st.baseline);
        const float ru = (un.fear[i] - un.baseline) / (0.90f - un.baseline);
        if (rs > 0.02f && rs < 0.98f && ru > 0.02f && ru < 0.98f) {
            ++compared;
            require(rs < ru, "stable agent must retain a smaller fraction of the fear excess than the unstable one at equal elapsed time");
        }
    }
    require(compared >= 3, "sampling must capture the decay window (otherwise adjust the grid)");

    // C3: path independence. Advancing in many steps == advancing in one jump.
    const std::size_t picks[] = {2, 8, 14, 20, 26};
    std::uint64_t local = 10;
    for (std::size_t k : picks) {
        if (k >= n1.ticks.size()) continue;
        AgentId id{WorldId(1), local++};
        make_fearful_agent(*r, id, neutral, 0);
        require(r->advance_to(id, PsychTime(n1.ticks[k])), "single-jump advance");
        require(std::fabs(fear_now(*r, id) - n1.fear[k]) < 1.0e-3f,
                "stepwise and single-jump advance must agree (lazy decay must be path independent)");
    }

    // C4: regression for the PsychTime(0) sentinel bug. Decay depends on elapsed time only.
    auto z = make_runtime(301);
    AgentId a0{WorldId(1), 1}, a1{WorldId(1), 2};
    make_fearful_agent(*z, a0, neutral, 0);
    make_fearful_agent(*z, a1, neutral, 1000);
    require(z->advance_to(a0, PsychTime(500)), "advance from t=0");
    require(z->advance_to(a1, PsychTime(1500)), "advance from t=1000");
    require(std::fabs(fear_now(*z, a0) - fear_now(*z, a1)) < 1.0e-6f,
            "decay must depend on elapsed time only, not on whether the start time is zero");
    require(fear_now(*z, a0) < 0.90f - 1.0e-4f, "fear must actually have decayed (guards against elapsed == 0)");
}

// ---------------------------------------------------------------------------
// P: personality changes decisions
// ---------------------------------------------------------------------------

struct Choice {
    bool risky;
    float margin;  // utility(risky) - utility(safe)
};

Choice choose(Runtime& r, std::uint64_t local, float boldness, float safe_extra_cost) {
    AgentId a{WorldId(1), local};
    AgentInit init{};
    Personality p{};
    p.boldness = boldness;  // only axis that differs between compared agents
    init.personality = p;
    require(r.create_agent(a, init, PsychTime(0)), "create profile agent");

    ActionOption safe{};
    safe.type = ActionTypeId(30);
    safe.instance = ActionInstanceId(1);
    safe.actor = a;
    safe.cost = 0.20f + safe_extra_cost;
    safe.risk = 0.05f;
    safe.context_stability = 1.0f;
    ActionOption risky = safe;
    risky.type = ActionTypeId(31);
    risky.instance = ActionInstanceId(2);
    risky.cost = 0.20f;
    risky.risk = 0.60f;

    auto d = r.decide(a, {safe, risky}, PsychTime(1));
    require(d && d.value_if()->has_selection, "profile decide must select");

    float us = 0.0f, ur = 0.0f;
    bool found_safe = false, found_risky = false;
    for (const auto& c : d.value_if()->trace) {
        if (c.action == ActionInstanceId(1)) { us = c.utility; found_safe = true; }
        if (c.action == ActionInstanceId(2)) { ur = c.utility; found_risky = true; }
    }
    require(found_safe && found_risky, "trace must list both candidates");
    return {d.value_if()->selected_action == ActionInstanceId(2), ur - us};
}

void test_profile_changes_decisions() {
    auto r = make_runtime(400);
    int divergent = 0;
    bool bold_was_risky = false, timid_was_risky = false;
    for (int k = 0; k <= 20; ++k) {
        const float extra = 0.03f * static_cast<float>(k);  // the safe option gets costlier
        const Choice bold = choose(*r, 1000 + 2 * k, 0.95f, extra);
        const Choice timid = choose(*r, 1001 + 2 * k, 0.05f, extra);

        // Primary relative claim: boldness raises the relative value of the risky option.
        require(bold.margin > timid.margin, "a bold agent must value the risky option more than a timid one");

        // No inversion of decisions.
        require(!(timid.risky && !bold.risky), "a timid agent must never accept risk that a bold agent refuses");

        // Monotone in the incentive: once risky is chosen, a costlier safe option keeps it chosen.
        if (bold_was_risky) require(bold.risky, "bold choice must be monotone in the incentive");
        if (timid_was_risky) require(timid.risky, "timid choice must be monotone in the incentive");
        bold_was_risky = bold_was_risky || bold.risky;
        timid_was_risky = timid_was_risky || timid.risky;

        if (bold.risky && !timid.risky) ++divergent;
    }
    require(divergent >= 1, "somewhere in the sweep, personality must flip the actual decision");
}

}  // namespace

int main() {
    test_hysteresis_does_not_lock();
    test_hysteresis_under_random_noise();
    test_decay_speed();
    test_profile_changes_decisions();
    std::cout << "AUDIT followup PASS\n";
    return 0;
}
