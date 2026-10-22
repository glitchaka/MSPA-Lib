#pragma once

#include "mspa/decision/decision.hpp"
#include "mspa/model/agent_state.hpp"

namespace mspa {

class HysteresisPolicy {
public:
    [[nodiscard]] float continuity_bonus(const AgentPsychState& agent,
                                         const ActionOption& option,
                                         ActionContinuityKey current) const noexcept;
};

} // namespace mspa
