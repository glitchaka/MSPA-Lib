#pragma once

#include "mspa/base/result.hpp"
#include "mspa/model/agent_state.hpp"
#include "mspa/semantics/registry.hpp"

namespace mspa {

struct AppraisalResult {
    float relevance{0.0f};
    float consequence{0.0f};
    float control{0.0f};
    float norm_alignment{0.0f};
};

class PsychologyEngine {
public:
    Result<bool> observe(
        AgentPsychState& agent,
        const Proposition& proposition,
        PsychTime process_time,
        PsychTime event_time,
        bool has_event_time,
        const RegistrySnapshot& registry,
        AppraisalResult* appraisal_out = nullptr) const;
};

} // namespace mspa
