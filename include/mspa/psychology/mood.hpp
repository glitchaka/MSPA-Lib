#pragma once

#include "mspa/model/agent_state.hpp"

namespace mspa {

class MoodSystem {
public:
    void update(AgentPsychState& agent, float valence_delta, float activation_delta, PsychTime now) const noexcept;
    void decay(AgentPsychState& agent, PsychTime now, float rate_per_tick = 0.0001f) const noexcept;
    void reset(AgentPsychState& agent, PsychTime now) const noexcept;
};

} // namespace mspa
