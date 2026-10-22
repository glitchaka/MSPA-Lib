#include "mspa/psychology/relationships.hpp"

#include <algorithm>

namespace mspa {

RelationshipState& RelationshipSystem::get_or_create(AgentPsychState& agent, AgentId other) const {
    auto it = std::find_if(agent.relationships.begin(), agent.relationships.end(),
                           [&](const RelationshipState& relation) { return relation.other == other; });
    if (it != agent.relationships.end()) return *it;
    agent.relationships.push_back(RelationshipState{other});
    return agent.relationships.back();
}

const RelationshipState* RelationshipSystem::find(const AgentPsychState& agent, AgentId other) const noexcept {
    auto it = std::find_if(agent.relationships.begin(), agent.relationships.end(),
                           [&](const RelationshipState& relation) { return relation.other == other; });
    return it == agent.relationships.end() ? nullptr : &*it;
}

} // namespace mspa
