#include "mspa/psychology/goals.hpp"

#include <algorithm>

namespace mspa {

Result<GoalId> GoalSystem::add(AgentPsychState& agent, ConceptId concept, float priority) const {
    if (!concept.valid()) return make_error(ErrorCode::InvalidArgument, "goal concept must be valid");
    std::uint64_t next = 1;
    for (const auto& goal : agent.goals) next = std::max(next, goal.id.value() + 1);
    GoalState state{};
    state.id = GoalId(next);
    state.concept = concept;
    state.priority = std::clamp(priority, 0.0f, 1.0f);
    state.active = true;
    agent.goals.push_back(state);
    return state.id;
}

Result<void> GoalSystem::complete(AgentPsychState& agent, GoalId goal_id) const {
    auto it = std::find_if(agent.goals.begin(), agent.goals.end(), [&](const GoalState& goal) { return goal.id == goal_id; });
    if (it == agent.goals.end()) return make_error(ErrorCode::InvalidArgument, "goal does not exist");
    it->active = false;
    return {};
}

} // namespace mspa
