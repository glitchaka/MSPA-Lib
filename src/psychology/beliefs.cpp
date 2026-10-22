#include "mspa/psychology/beliefs.hpp"

#include <algorithm>

namespace mspa {

Result<BeliefId> BeliefSystem::add(AgentPsychState& agent, PredicateId predicate, float certainty,
                                   float emotional_load, PsychTime updated_at) const {
    if (!agent.id.valid() || !predicate.valid()) {
        return make_error(ErrorCode::InvalidArgument, "agent and predicate must be valid");
    }
    std::uint64_t next = 1;
    for (const auto& belief : agent.beliefs) {
        next = std::max(next, belief.id.value() + 1);
    }
    BeliefState state{};
    state.id = BeliefId(next);
    state.predicate = predicate;
    state.certainty = std::clamp(certainty, 0.0f, 1.0f);
    state.emotional_load = std::clamp(emotional_load, -1.0f, 1.0f);
    state.updated_at = updated_at;
    agent.beliefs.push_back(state);
    return state.id;
}

Result<void> BeliefSystem::reinforce(AgentPsychState& agent, BeliefId belief_id, float evidence) const {
    auto it = std::find_if(agent.beliefs.begin(), agent.beliefs.end(),
                           [&](const BeliefState& belief) { return belief.id == belief_id; });
    if (it == agent.beliefs.end()) {
        return make_error(ErrorCode::InvalidArgument, "belief does not exist");
    }
    it->certainty = std::clamp(it->certainty + evidence, 0.0f, 1.0f);
    return {};
}

} // namespace mspa
