#pragma once

#include "mspa/base/result.hpp"
#include "mspa/model/agent_state.hpp"

namespace mspa {

class MemorySystem {
public:
    Result<MemoryId> remember(AgentPsychState& agent, PredicateId predicate, float importance,
                              PsychTime event_time) const;
    void compact(AgentPsychState& agent, std::size_t max_memories) const;
};

} // namespace mspa
