#pragma once

#include "mspa/model/agent_state.hpp"

namespace mspa {

class LearningSystem {
public:
    void report(AgentPsychState& agent, ActionTypeId action, bool success) const;
    [[nodiscard]] float expected_success(const AgentPsychState& agent, ActionTypeId action) const noexcept;
};

} // namespace mspa
