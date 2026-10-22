#pragma once

#include "mspa/model/agent_state.hpp"
#include "mspa/psychology/appraisal.hpp"

namespace mspa {

class EmotionSystem {
public:
    void apply(AgentPsychState& agent, const AppraisalVector& appraisal, PsychTime now) const;
    void decay(AgentPsychState& agent, PsychTime now) const;
};

} // namespace mspa
