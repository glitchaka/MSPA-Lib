#include "test.hpp"
#include "mspa/mspa.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace mspa;

namespace {

RegistrySnapshot phase2_registry() {
    RegistryBuilder b;
    PredicateSchema threat{};
    threat.id = PredicateId(1);
    threat.name_space = NamespaceId(1);
    threat.default_relevance = 0.80f;
    threat.default_consequence = -0.60f;
    threat.appraisal_hints = {"threat"};
    require(b.add_predicate(threat), "register threat predicate");

    PredicateSchema neutral{};
    neutral.id = PredicateId(2);
    neutral.name_space = NamespaceId(1);
    neutral.default_relevance = 0.0f;
    neutral.default_consequence = 0.0f;
    require(b.add_predicate(neutral), "register neutral predicate");

    PredicateSchema belief{};
    belief.id = PredicateId(3);
    belief.name_space = NamespaceId(1);
    belief.default_relevance = 0.20f;
    belief.default_consequence = 0.0f;
    require(b.add_predicate(belief), "register belief predicate");
    return b.publish();
}

std::unique_ptr<Runtime> make_runtime(std::uint64_t runtime_id, bool diagnostics = true,
                                      const RegistrySnapshot& registry = RegistrySnapshot{}) {
    RuntimeConfig cfg{};
    cfg.runtime_id = RuntimeId(runtime_id);
    cfg.world_id = WorldId(1);
    cfg.time_domain = {TimeDomainId(1), {60, 1}};
    cfg.seed_root = 0x9e3779b97f4a7c15ULL;
    cfg.diagnostics_enabled = diagnostics;
    auto created = Runtime::create(cfg, registry);
    require(created, "create phase2 behavior runtime");
    return std::move(*created.value_if());
}

PsychologicalProfile make_profile(std::uint64_t id, const char* name, const Personality& p) {
    PsychologicalProfile profile{};
    profile.id = ProfileId(id);
    profile.name = name;
    profile.personality = p;
    return profile;
}

ObserveInput observation(AgentId agent, PredicateId predicate, bool positive, std::uint64_t process_time,
                         float certainty = 0.80f, float trust = 0.80f) {
    ObserveInput input{};
    input.agent = agent;
    input.proposition.predicate = predicate;
    input.proposition.name_space = NamespaceId(1);
    input.proposition.positive = positive;
    input.time.process_time = PsychTime(process_time);
    input.perceived_certainty = certainty;
    input.source_trust = trust;
    return input;
}

const BeliefState* belief_for(const AgentPsychState& state, PredicateId predicate) {
    const auto it = std::find_if(state.beliefs.begin(), state.beliefs.end(),
                                 [&](const BeliefState& belief) { return belief.proposition.predicate == predicate; });
    return it == state.beliefs.end() ? nullptr : &*it;
}

float emotion_of(const AgentPsychState& state, BasicEmotion kind) {
    const auto it = std::find_if(state.emotions.begin(), state.emotions.end(),
                                 [&](const EmotionState& emotion) { return emotion.kind == kind; });
    return it == state.emotions.end() ? 0.0f : it->intensity;
}

const LearnedExpectation* expectation_for(const AgentPsychState& state, ActionTypeId action,
                                           const std::string& context, const std::string& outcome = "success") {
    const auto it = std::find_if(state.learned_expectations.begin(), state.learned_expectations.end(),
                                 [&](const LearnedExpectation& e) {
                                     return e.action == action && e.outcome_tag == outcome &&
                                            e.context_tags.size() == 1 && e.context_tags.front() == context;
                                 });
    return it == state.learned_expectations.end() ? nullptr : &*it;
}

std::vector<ActionOption> near_tie(AgentId agent, bool continuity, bool flip) {
    ActionOption a{};
    a.type = ActionTypeId(1);
    a.instance = ActionInstanceId(1);
    a.continuity = continuity ? ActionContinuityKey(1) : ActionContinuityKey{};
    a.actor = agent;
    a.cost = flip ? 0.11f : 0.10f;
    a.progress = 0.35f;
    a.interruption_cost = 0.30f;
    a.context_stability = 1.0f;

    ActionOption b = a;
    b.instance = ActionInstanceId(2);
    b.continuity = continuity ? ActionContinuityKey(2) : ActionContinuityKey{};
    b.cost = flip ? 0.10f : 0.11f;
    return {a, b};
}

int switch_count(bool continuity) {
    auto runtime = make_runtime(continuity ? 300 : 301);
    const AgentId agent{WorldId(1), 1};
    AgentInit init{};
    init.personality.persistence = 0.85f;
    init.personality.stability = 0.85f;
    require(runtime->create_agent(agent, init, PsychTime(0)), "create hysteresis phase2 agent");

    std::uint64_t previous = 0;
    int switches = 0;
    for (int i = 0; i < 100; ++i) {
        auto decision = runtime->decide(agent, near_tie(agent, continuity, (i % 2) != 0),
                                        PsychTime(static_cast<std::uint64_t>(i + 1)));
        require(decision && decision.value_if()->has_selection, "phase2 hysteresis decision");
        const auto selected = decision.value_if()->selected_action.value();
        if (previous != 0 && selected != previous) ++switches;
        previous = selected;
    }
    return switches;
}

void assert_trace_reproduces_selection(const Decision& decision) {
    require(decision.has_selection, "trace decision has selection");
    float best = -1.0e30f;
    ActionInstanceId best_action{};
    for (const auto& candidate : decision.trace) {
        float sum = 0.0f;
        for (const auto& contribution : candidate.considerations) sum += contribution.weighted;
        require(std::fabs(sum - candidate.utility) < 1.0e-5f,
                "candidate contributions reproduce utility");
        if (candidate.utility > best ||
            (candidate.utility == best && (!best_action.valid() || candidate.action < best_action))) {
            best = candidate.utility;
            best_action = candidate.action;
        }
    }
    require(best_action == decision.selected_action, "trace winner reproduces selected action");
}

struct DiagnosticScenarioResult {
    std::vector<std::uint8_t> bytes{};
    ActionInstanceId selected{};
    RngState rng{};
    std::size_t diagnostic_count{0};
};

DiagnosticScenarioResult diagnostic_scenario(bool enabled, const RegistrySnapshot& registry) {
    auto runtime = make_runtime(enabled ? 320 : 321, enabled, registry);
    const AgentId agent{WorldId(1), 90};
    require(runtime->create_agent(agent, AgentInit{}, PsychTime(0)), "diagnostic scenario create");
    require(runtime->observe(observation(agent, PredicateId(1), true, 10)), "diagnostic scenario observe");

    ActionOption a{};
    a.type = ActionTypeId(20);
    a.instance = ActionInstanceId(20);
    a.actor = agent;
    a.cost = 0.15f;
    a.risk = 0.10f;
    a.progress = 0.65f;
    ActionOption b = a;
    b.type = ActionTypeId(21);
    b.instance = ActionInstanceId(21);
    b.cost = 0.20f;
    b.risk = 0.05f;
    b.progress = 0.45f;

    auto decision = runtime->decide(agent, {a, b}, PsychTime(11));
    require(decision, "diagnostic scenario decide");
    assert_trace_reproduces_selection(*decision.value_if());

    ReportInput report{};
    report.agent = agent;
    report.action_type = ActionTypeId(20);
    report.success = true;
    report.context_tags = {"phase2"};
    report.process_time = PsychTime(12);
    require(runtime->report(report), "diagnostic scenario report");
    require(runtime->advance_to(agent, PsychTime(100)), "diagnostic scenario advance");

    auto exported = runtime->export_agent(agent, PsychTime(100));
    require(exported, "diagnostic scenario export state");
    auto bytes = runtime->export_agent_bytes(agent, PsychTime(100));
    require(bytes, "diagnostic scenario export bytes");

    DiagnosticScenarioResult result{};
    result.bytes = *bytes.value_if();
    result.selected = decision.value_if()->selected_action;
    result.rng = exported.value_if()->state.rng_state;
    result.diagnostic_count = runtime->diagnostics().traces().size();
    return result;
}

} // namespace

int main() {
    const auto registry = phase2_registry();

    // A2: same evidence, four profiles, only relative assertions.
    ProfileRegistry profiles;
    Personality rigid{};
    rigid.persistence = 0.95f;
    rigid.curiosity = 0.05f;
    rigid.stability = 0.50f;
    rigid.boldness = 0.50f;
    Personality flexible = rigid;
    flexible.persistence = 0.05f;
    flexible.curiosity = 0.95f;
    Personality stable = rigid;
    stable.persistence = 0.50f;
    stable.curiosity = 0.50f;
    stable.stability = 0.95f;
    stable.boldness = 0.85f;
    Personality unstable = stable;
    unstable.stability = 0.10f;
    unstable.boldness = 0.15f;

    require(profiles.add(make_profile(1, "rigid", rigid)), "add rigid phase2 profile");
    require(profiles.add(make_profile(2, "flexible", flexible)), "add flexible phase2 profile");
    require(profiles.add(make_profile(3, "stable", stable)), "add stable phase2 profile");
    require(profiles.add(make_profile(4, "unstable", unstable)), "add unstable phase2 profile");

    auto profile_runtime = make_runtime(302, true, registry);
    const AgentId profile_ids[4] = {{WorldId(1), 1}, {WorldId(1), 2}, {WorldId(1), 3}, {WorldId(1), 4}};
    for (std::size_t i = 0; i < 4; ++i) {
        ProfileRecipe recipe{};
        recipe.components.push_back({ProfileId(static_cast<std::uint64_t>(i + 1)), 1.0f});
        require(create_agent_from_profiles(*profile_runtime, profile_ids[i], profiles, recipe, PsychTime(0)),
                "create phase2 profiled agent");
        require(profile_runtime->observe(observation(profile_ids[i], PredicateId(1), true, 10)),
                "observe same evidence for profile");
    }

    auto rigid_export = profile_runtime->export_agent(profile_ids[0], PsychTime(10));
    auto flexible_export = profile_runtime->export_agent(profile_ids[1], PsychTime(10));
    auto stable_export = profile_runtime->export_agent(profile_ids[2], PsychTime(10));
    auto unstable_export = profile_runtime->export_agent(profile_ids[3], PsychTime(10));
    require(rigid_export && flexible_export && stable_export && unstable_export, "export four profile states");

    const auto* rigid_belief = belief_for(rigid_export.value_if()->state, PredicateId(1));
    const auto* flexible_belief = belief_for(flexible_export.value_if()->state, PredicateId(1));
    require(rigid_belief && flexible_belief, "profile beliefs exist");
    require(rigid_belief->certainty < flexible_belief->certainty,
            "higher rigidity must accept identical evidence less readily");
    require(emotion_of(stable_export.value_if()->state, BasicEmotion::Fear) <
                emotion_of(unstable_export.value_if()->state, BasicEmotion::Fear),
            "stable-bold profile must show less fear under identical threat evidence");

    auto recovery = make_runtime(303);
    const AgentId stable_id{WorldId(1), 30};
    const AgentId unstable_id{WorldId(1), 31};
    AgentInit stable_init{};
    stable_init.personality = stable;
    stable_init.emotions.push_back({BasicEmotion::Fear, 0.90f, PsychTime(0), PsychTime(0)});
    AgentInit unstable_init = stable_init;
    unstable_init.personality = unstable;
    require(recovery->create_agent(stable_id, stable_init, PsychTime(0)), "create stable recovery agent");
    require(recovery->create_agent(unstable_id, unstable_init, PsychTime(0)), "create unstable recovery agent");
    require(recovery->advance_to(stable_id, PsychTime(1000)), "advance stable recovery agent");
    require(recovery->advance_to(unstable_id, PsychTime(1000)), "advance unstable recovery agent");
    auto stable_recovered = recovery->export_agent(stable_id, PsychTime(1000));
    auto unstable_recovered = recovery->export_agent(unstable_id, PsychTime(1000));
    require(stable_recovered && unstable_recovered, "export recovered profile states");
    require(emotion_of(stable_recovered.value_if()->state, BasicEmotion::Fear) <
                emotion_of(unstable_recovered.value_if()->state, BasicEmotion::Fear),
            "higher stability must recover fear faster");

    // A3: frozen threshold: <=10 switches and at least 3x fewer than without continuity.
    const int with_hysteresis = switch_count(true);
    const int without_hysteresis = switch_count(false);
    require(with_hysteresis <= 10, "A3 continuity switch count must be <=10");
    require(with_hysteresis * 3 <= without_hysteresis,
            "A3 continuity must reduce switching by at least factor three");
    require(without_hysteresis > with_hysteresis, "A3 perturbation must expose measurable oscillation");

    // A4: REPORT success/failure changes learned expectations and DECIDE uses them.
    auto learning = make_runtime(304);
    const AgentId learner{WorldId(1), 40};
    require(learning->create_agent(learner, AgentInit{}, PsychTime(0)), "create phase2 learner");
    for (std::uint64_t i = 1; i <= 8; ++i) {
        ReportInput good{};
        good.agent = learner;
        good.action_type = ActionTypeId(10);
        good.success = true;
        good.context_tags = {"rain"};
        good.process_time = PsychTime(i);
        require(learning->report(good), "report repeated success");

        ReportInput bad = good;
        bad.action_type = ActionTypeId(11);
        bad.success = false;
        require(learning->report(bad), "report repeated failure");
    }
    auto learned = learning->export_agent(learner, PsychTime(8));
    require(learned, "export learned expectations");
    const auto* good_expectation = expectation_for(learned.value_if()->state, ActionTypeId(10), "rain");
    const auto* bad_expectation = expectation_for(learned.value_if()->state, ActionTypeId(11), "rain");
    require(good_expectation && bad_expectation, "success expectations exist");
    require(good_expectation->confidence > 0.75f, "success must raise expectation");
    require(bad_expectation->confidence < 0.25f, "failure must lower success expectation");

    ActionOption learned_good{};
    learned_good.type = ActionTypeId(10);
    learned_good.instance = ActionInstanceId(10);
    learned_good.actor = learner;
    learned_good.context_tags = {"rain"};
    ActionOption learned_bad = learned_good;
    learned_bad.type = ActionTypeId(11);
    learned_bad.instance = ActionInstanceId(11);
    auto learned_decision = learning->decide(learner, {learned_bad, learned_good}, PsychTime(9));
    require(learned_decision && learned_decision.value_if()->selected_action == ActionInstanceId(10),
            "DECIDE must prefer repeatedly successful action");

    for (std::uint64_t i = 10; i <= 17; ++i) {
        ReportInput rain{};
        rain.agent = learner;
        rain.action_type = ActionTypeId(12);
        rain.success = true;
        rain.context_tags = {"rain"};
        rain.process_time = PsychTime(i);
        require(learning->report(rain), "learn rain context");
        ReportInput sun = rain;
        sun.success = false;
        sun.context_tags = {"sun"};
        require(learning->report(sun), "learn sun context");
    }
    auto contextual = learning->export_agent(learner, PsychTime(17));
    require(contextual, "export contextual learning state");
    const auto* rain_expectation = expectation_for(contextual.value_if()->state, ActionTypeId(12), "rain");
    const auto* sun_expectation = expectation_for(contextual.value_if()->state, ActionTypeId(12), "sun");
    require(rain_expectation && sun_expectation, "context-specific expectations exist");
    require(rain_expectation->confidence > sun_expectation->confidence + 0.50f,
            "ContextTags must specialize learned expectation");

    // A5: contradiction lowers certainty, emotional load/rigidity slow change, enough evidence flips polarity.
    auto beliefs_runtime = make_runtime(305, true, registry);
    const AgentId low_load{WorldId(1), 50};
    const AgentId high_load{WorldId(1), 51};
    Proposition fact{};
    fact.predicate = PredicateId(3);
    fact.name_space = NamespaceId(1);
    fact.positive = true;
    BeliefState low_belief{};
    low_belief.id = BeliefId(1);
    low_belief.proposition = fact;
    low_belief.certainty = 0.90f;
    low_belief.emotional_load = 0.0f;
    low_belief.updated_at = PsychTime(1);
    BeliefState high_belief = low_belief;
    high_belief.emotional_load = 0.95f;
    AgentInit low_init{};
    low_init.beliefs.push_back(low_belief);
    AgentInit high_init{};
    high_init.beliefs.push_back(high_belief);
    require(beliefs_runtime->create_agent(low_load, low_init, PsychTime(1)), "create low-load belief agent");
    require(beliefs_runtime->create_agent(high_load, high_init, PsychTime(1)), "create high-load belief agent");
    require(beliefs_runtime->observe(observation(low_load, PredicateId(3), false, 2, 0.50f, 1.0f)),
            "contradict low-load belief");
    require(beliefs_runtime->observe(observation(high_load, PredicateId(3), false, 2, 0.50f, 1.0f)),
            "contradict high-load belief");
    auto low_after_one = beliefs_runtime->export_agent(low_load, PsychTime(2));
    auto high_after_one = beliefs_runtime->export_agent(high_load, PsychTime(2));
    require(low_after_one && high_after_one, "export contradictory belief states");
    const auto* low_one = belief_for(low_after_one.value_if()->state, PredicateId(3));
    const auto* high_one = belief_for(high_after_one.value_if()->state, PredicateId(3));
    require(low_one && high_one, "contradictory beliefs remain represented");
    require(low_one->certainty < 0.90f && high_one->certainty < 0.90f,
            "contradictory evidence must reduce prior certainty before flip");
    require(high_one->certainty > low_one->certainty,
            "higher emotional load must resist contradictory evidence more strongly");

    bool low_flipped = !low_one->proposition.positive;
    bool high_flipped = !high_one->proposition.positive;
    for (std::uint64_t t = 3; t <= 30 && (!low_flipped || !high_flipped); ++t) {
        if (!low_flipped) {
            require(beliefs_runtime->observe(observation(low_load, PredicateId(3), false, t, 0.50f, 1.0f)),
                    "repeat low-load contradiction");
            auto state = beliefs_runtime->export_agent(low_load, PsychTime(t));
            require(state, "export low-load contradiction loop");
            low_flipped = !belief_for(state.value_if()->state, PredicateId(3))->proposition.positive;
        }
        if (!high_flipped) {
            require(beliefs_runtime->observe(observation(high_load, PredicateId(3), false, t, 0.50f, 1.0f)),
                    "repeat high-load contradiction");
            auto state = beliefs_runtime->export_agent(high_load, PsychTime(t));
            require(state, "export high-load contradiction loop");
            high_flipped = !belief_for(state.value_if()->state, PredicateId(3))->proposition.positive;
        }
    }
    require(low_flipped && high_flipped, "sufficient contradictory support must invert belief polarity");

    auto rigidity_runtime = make_runtime(306, true, registry);
    const AgentId rigid_id{WorldId(1), 52};
    const AgentId flexible_id{WorldId(1), 53};
    AgentInit rigid_init{};
    rigid_init.personality = rigid;
    rigid_init.beliefs.push_back(low_belief);
    AgentInit flexible_init{};
    flexible_init.personality = flexible;
    flexible_init.beliefs.push_back(low_belief);
    require(rigidity_runtime->create_agent(rigid_id, rigid_init, PsychTime(1)), "create rigid belief agent");
    require(rigidity_runtime->create_agent(flexible_id, flexible_init, PsychTime(1)), "create flexible belief agent");
    require(rigidity_runtime->observe(observation(rigid_id, PredicateId(3), false, 2, 0.50f, 1.0f)),
            "contradict rigid belief");
    require(rigidity_runtime->observe(observation(flexible_id, PredicateId(3), false, 2, 0.50f, 1.0f)),
            "contradict flexible belief");
    auto rigid_after = rigidity_runtime->export_agent(rigid_id, PsychTime(2));
    auto flexible_after = rigidity_runtime->export_agent(flexible_id, PsychTime(2));
    require(rigid_after && flexible_after, "export rigidity states");
    require(belief_for(rigid_after.value_if()->state, PredicateId(3))->certainty >
                belief_for(flexible_after.value_if()->state, PredicateId(3))->certainty,
            "higher rigidity must slow contradictory change");

    AgentPsychState reappraised{};
    reappraised.id = {WorldId(1), 60};
    MemoryState memory{};
    memory.id = MemoryId(1);
    memory.proposition = fact;
    memory.importance = 0.80f;
    memory.emotional_load = -0.80f;
    memory.certainty = 0.90f;
    reappraised.memories.push_back(memory);
    const Proposition before_reappraisal = reappraised.memories.front().proposition;
    require(ReappraisalSystem{}.reappraise_memory(reappraised, MemoryId(1), 0.20f, 0.70f),
            "reappraise public memory API");
    require(equivalent(before_reappraisal, reappraised.memories.front().proposition),
            "reappraisal must preserve factual proposition");

    // A6: all six baselines survive long time and are behaviorally tonic, not episodic.
    auto baseline_runtime = make_runtime(307, true, registry);
    const AgentId baseline_agent{WorldId(1), 70};
    require(baseline_runtime->create_agent(baseline_agent, AgentInit{}, PsychTime(0)), "create baseline phase2 agent");
    auto advanced = baseline_runtime->advance_to(baseline_agent, PsychTime(1000000));
    require(advanced && !advanced.value_if()->decision_recommended,
            "tonic baseline alone must not recommend urgent reevaluation");
    auto baseline_state = baseline_runtime->export_agent(baseline_agent, PsychTime(1000000));
    require(baseline_state, "export long-advanced baseline state");
    require(baseline_state.value_if()->state.emotions.size() == 6, "all six base emotions remain after long advance");
    for (const auto& emotion : baseline_state.value_if()->state.emotions)
        require(emotion.intensity >= 0.01f, "every tonic baseline remains at least one percent");

    auto neutral_result = baseline_runtime->observe(observation(baseline_agent, PredicateId(2), true, 1000001));
    require(neutral_result, "observe neutral stimulus after long baseline");
    require(neutral_result.value_if()->urgency != Urgency::Soon &&
                neutral_result.value_if()->urgency != Urgency::Immediate,
            "tonic baseline alone must not promote urgency to Soon/Immediate");

    auto fresh_runtime = make_runtime(308, true, registry);
    const AgentId fresh_agent{WorldId(1), 71};
    require(fresh_runtime->create_agent(fresh_agent, AgentInit{}, PsychTime(0)), "create fresh comparison agent");
    ActionOption safer{};
    safer.type = ActionTypeId(30);
    safer.instance = ActionInstanceId(30);
    safer.actor = baseline_agent;
    safer.cost = 0.10f;
    safer.risk = 0.10f;
    safer.progress = 0.30f;
    ActionOption riskier = safer;
    riskier.type = ActionTypeId(31);
    riskier.instance = ActionInstanceId(31);
    riskier.cost = 0.05f;
    riskier.risk = 0.35f;
    riskier.progress = 0.55f;
    auto baseline_decision = baseline_runtime->decide(baseline_agent, {safer, riskier}, PsychTime(1000002));
    safer.actor = fresh_agent;
    riskier.actor = fresh_agent;
    auto fresh_decision = fresh_runtime->decide(fresh_agent, {safer, riskier}, PsychTime(1000002));
    require(baseline_decision && fresh_decision, "compare tonic baseline decisions");
    require(baseline_decision.value_if()->selected_action == fresh_decision.value_if()->selected_action,
            "long-lived tonic baseline alone must not change action choice");

    // A7: trace is arithmetic and diagnostics are observational only.
    const auto diagnostics_on = diagnostic_scenario(true, registry);
    const auto diagnostics_off = diagnostic_scenario(false, registry);
    require(diagnostics_on.selected == diagnostics_off.selected,
            "diagnostics toggle must not change selected action");
    require(diagnostics_on.bytes == diagnostics_off.bytes,
            "diagnostics toggle must not change exported psychological state");
    require(diagnostics_on.rng.state == diagnostics_off.rng.state,
            "diagnostics toggle must not consume agent RNG");
    require(diagnostics_on.diagnostic_count > 0, "enabled diagnostics must produce traces");
    require(diagnostics_off.diagnostic_count == 0, "disabled diagnostics must not retain traces");

    std::cout << "AUDIT_PHASE2_BEHAVIOR PASS"
              << " certainty_order=" << rigid_belief->certainty << "<" << flexible_belief->certainty
              << " switches=" << with_hysteresis << "/" << without_hysteresis
              << " learning=" << good_expectation->confidence << "/" << bad_expectation->confidence
              << " diagnostics=" << diagnostics_on.diagnostic_count << "/" << diagnostics_off.diagnostic_count
              << "\n";
}
