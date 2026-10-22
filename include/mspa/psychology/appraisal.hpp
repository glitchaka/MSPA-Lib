#pragma once

#include "mspa/model/agent_state.hpp"
#include "mspa/semantics/registry.hpp"

namespace mspa {

struct AppraisalVector {
    float relevance{0.0f};
    float consequence{0.0f};
    float agency{0.0f};
    float control{0.0f};
    float norm_alignment{0.0f};
    float expectation_error{0.0f};
};

class AppraisalEngine {
public:
    [[nodiscard]] AppraisalVector evaluate(const AgentPsychState& agent,
                                           const Proposition& proposition,
                                           const RegistrySnapshot& registry) const;
};

} // namespace mspa
