#pragma once

#include "mspa/base/result.hpp"
#include "mspa/model/agent_state.hpp"

namespace mspa {

class GoalSystem {
public:
    Result<GoalId> add(AgentPsychState& agent, ConceptId concept, float priority) const;
    Result<void> complete(AgentPsychState& agent, GoalId goal) const;
};

} // namespace mspa
