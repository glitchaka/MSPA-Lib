#include "mspa/validation/limits.hpp"

namespace mspa {

Result<void> validate_limits(const RuntimeLimits& limits) {
    if (limits.max_agents == 0 || limits.max_agents > HardLimits::max_agents ||
        limits.max_beliefs_per_agent == 0 || limits.max_beliefs_per_agent > HardLimits::max_beliefs_per_agent ||
        limits.max_memories_per_agent == 0 || limits.max_memories_per_agent > HardLimits::max_memories_per_agent ||
        limits.max_goals_per_agent == 0 || limits.max_goals_per_agent > HardLimits::max_goals_per_agent ||
        limits.max_relationships_per_agent == 0 || limits.max_relationships_per_agent > HardLimits::max_relationships_per_agent) {
        return make_error(ErrorCode::InvalidArgument, "runtime limits must be non-zero and within hard limits");
    }
    return {};
}

Result<void> validate_agent_state(const AgentPsychState& state, const RuntimeLimits& limits) {
    if (!state.id.valid()) {
        return make_error(ErrorCode::InvalidState, "agent id is invalid");
    }
    if (state.beliefs.size() > limits.max_beliefs_per_agent ||
        state.memories.size() > limits.max_memories_per_agent ||
        state.goals.size() > limits.max_goals_per_agent ||
        state.relationships.size() > limits.max_relationships_per_agent) {
        return make_error(ErrorCode::QuotaExceeded, "agent state exceeds runtime limits");
    }
    return {};
}

} // namespace mspa
