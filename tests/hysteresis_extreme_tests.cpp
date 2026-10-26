#include "test.hpp"
#include "mspa/mspa.hpp"
#include "mspa/decision/hysteresis.hpp"
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

using namespace mspa;

namespace {
std::unique_ptr<Runtime> make_runtime(std::uint64_t rid) {
    RuntimeConfig config{};
    config.runtime_id = RuntimeId(rid);
    config.world_id = WorldId(1);
    config.time_domain = {TimeDomainId(1), {1, 1}};
    config.seed_root = 77;
    auto created = Runtime::create(config);
    require(created, "create hysteresis runtime");
    return std::move(*created.value_if());
}

std::vector<ActionOption> make_options(AgentId actor, float gap, bool extreme) {
    ActionOption incumbent{};
    incumbent.type = ActionTypeId(1);
    incumbent.instance = ActionInstanceId(1);
    incumbent.actor = actor;
    incumbent.continuity = ActionContinuityKey(1);
    incumbent.cost = 0.60f;
    incumbent.progress = extreme ? 1.0f : 0.35f;
    incumbent.interruption_cost = extreme ? 1.0f : 0.30f;
    incumbent.context_stability = 1.0f;

    // Keep every base-utility term identical between candidates except cost.
    // That makes `gap` an actual pre-hysteresis utility advantage instead of
    // accidentally mixing progress/interruption differences into the test.
    ActionOption better = incumbent;
    better.type = ActionTypeId(2);
    better.instance = ActionInstanceId(2);
    better.continuity = ActionContinuityKey(2);
    better.cost = 0.60f - gap;
    return {incumbent, better};
}

int latency(float gap, bool extreme, std::uint64_t rid) {
    auto runtime = make_runtime(rid);
    AgentId agent{WorldId(1), 1};
    AgentInit init{};
    init.personality.persistence = extreme ? 1.0f : 0.85f;
    init.personality.stability = extreme ? 1.0f : 0.85f;
    require(runtime->create_agent(agent, init, PsychTime(0)), "create hysteresis agent");

    for (std::uint64_t t = 1; t <= 20; ++t) {
        auto warm = runtime->decide(agent, make_options(agent, -0.01f, extreme), PsychTime(t));
        require(warm && warm.value_if()->selected_action == ActionInstanceId(1), "warmup establishes incumbent");
    }
    for (int step = 1; step <= 40; ++step) {
        auto decision = runtime->decide(agent, make_options(agent, gap, extreme), PsychTime(20 + step));
        require(decision && decision.value_if()->has_selection, "hysteresis decision exists");
        if (decision.value_if()->selected_action == ActionInstanceId(2)) return step;
    }
    return 41;
}
} // namespace

int main() {
    AgentPsychState extreme_state{};
    extreme_state.id = {WorldId(1), 1};
    extreme_state.personality.persistence = 1.0f;
    extreme_state.personality.stability = 1.0f;
    ActionOption extreme_option{};
    extreme_option.type = ActionTypeId(1);
    extreme_option.instance = ActionInstanceId(1);
    extreme_option.actor = extreme_state.id;
    extreme_option.continuity = ActionContinuityKey(1);
    extreme_option.progress = 1.0f;
    extreme_option.interruption_cost = 1.0f;
    extreme_option.context_stability = 1.0f;

    HysteresisPolicy policy;
    const float bonus = policy.continuity_bonus(extreme_state, extreme_option, ActionContinuityKey(1), 0.0f);
    const float guard = policy.switch_threshold(extreme_state, extreme_option, 0.0f);
    require(bonus + guard <= 0.350001f, "total hysteresis inertia must be hard-capped at 0.35");
    require(policy.continuity_bonus(extreme_state, extreme_option, ActionContinuityKey(1), 1.0f) == 0.0f,
            "immediate urgency cancels continuity inertia");
    require(policy.switch_threshold(extreme_state, extreme_option, 1.0f) == 0.0f,
            "immediate urgency cancels switch guard");

    const float gaps[] = {0.30f, 0.35f, 0.40f, 0.45f};
    std::cout << "sticky latency table";
    for (std::size_t i = 0; i < 4; ++i) {
        const int value = latency(gaps[i], false, 100 + i);
        std::cout << " gap=" << gaps[i] << ':' << value;
        if (gaps[i] >= 0.40f)
            require(value <= 3, "sticky agent must adopt a >=0.40 improvement promptly");
    }
    std::cout << '\n';

    const int extreme_040 = latency(0.40f, true, 200);
    const int extreme_045 = latency(0.45f, true, 201);
    require(extreme_040 <= 3, "extreme personality must not block a 0.40 improvement");
    require(extreme_045 <= extreme_040, "larger improvement must not be adopted later");

    std::cout << "hysteresis extremes ok total=" << (bonus + guard)
              << " extreme0.40=" << extreme_040
              << " extreme0.45=" << extreme_045 << '\n';
}
