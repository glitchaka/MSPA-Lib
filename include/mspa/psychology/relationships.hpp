#pragma once

#include "mspa/model/agent_state.hpp"

namespace mspa {

class RelationshipSystem {
public:
    RelationshipState& get_or_create(AgentPsychState& agent, AgentId other) const;
    const RelationshipState* find(const AgentPsychState& agent, AgentId other) const noexcept;
};

} // namespace mspa
