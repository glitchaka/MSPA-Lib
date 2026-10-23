#pragma once

#include "mspa/base/result.hpp"
#include "mspa/model/agent_state.hpp"

namespace mspa {

class ValueSystem {
public:
    Result<void> set(AgentPsychState& agent, ConceptId concept_id, float intensity) const;
    Result<void> erase(AgentPsychState& agent, ConceptId concept_id) const;
    [[nodiscard]] float intensity(const AgentPsychState& agent, ConceptId concept_id) const noexcept;
};

} // namespace mspa
