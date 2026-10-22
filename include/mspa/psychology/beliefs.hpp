#pragma once

#include "mspa/base/result.hpp"
#include "mspa/model/agent_state.hpp"

namespace mspa {

class BeliefSystem {
public:
    Result<BeliefId> add(AgentPsychState& agent, PredicateId predicate, float certainty,
                         float emotional_load, PsychTime updated_at) const;
    Result<void> reinforce(AgentPsychState& agent, BeliefId belief, float evidence) const;
};

} // namespace mspa
