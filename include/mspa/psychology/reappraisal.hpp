#pragma once

#include "mspa/base/result.hpp"
#include "mspa/model/agent_state.hpp"

namespace mspa {

class ReappraisalSystem {
public:
    Result<void> reappraise_memory(AgentPsychState& agent, MemoryId memory,
                                    float emotional_load, float certainty) const;
    Result<void> reactivate_memory(AgentPsychState& agent, MemoryId memory) const;
};

} // namespace mspa
