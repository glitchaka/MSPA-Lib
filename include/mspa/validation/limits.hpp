#pragma once

#include <cstddef>

#include "mspa/base/result.hpp"
#include "mspa/model/agent_state.hpp"

namespace mspa {

struct RuntimeLimits {
    std::size_t max_agents{100000};
    std::size_t max_beliefs_per_agent{4096};
    std::size_t max_memories_per_agent{4096};
    std::size_t max_goals_per_agent{256};
    std::size_t max_relationships_per_agent{4096};
};

struct HardLimits {
    static constexpr std::size_t max_agents = 1000000;
    static constexpr std::size_t max_beliefs_per_agent = 65536;
    static constexpr std::size_t max_memories_per_agent = 65536;
    static constexpr std::size_t max_goals_per_agent = 4096;
    static constexpr std::size_t max_relationships_per_agent = 65536;
};

Result<void> validate_limits(const RuntimeLimits& limits);
Result<void> validate_agent_state(const AgentPsychState& state, const RuntimeLimits& limits);

} // namespace mspa
