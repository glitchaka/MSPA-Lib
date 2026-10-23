#include "mspa/psychology/values.hpp"

#include <algorithm>

namespace mspa {

Result<void> ValueSystem::set(AgentPsychState& agent, ConceptId concept_id, float value) const {
    if (!concept_id.valid()) return make_error(ErrorCode::InvalidArgument, "value concept id must be valid");
    const float normalized = std::clamp(value, 0.0f, 1.0f);
    auto it = std::find_if(agent.values.begin(), agent.values.end(), [&](const ValueEntry& entry) {
        return entry.concept_id == concept_id;
    });
    if (it == agent.values.end()) agent.values.push_back({concept_id, normalized});
    else it->intensity = normalized;
    return {};
}

Result<void> ValueSystem::erase(AgentPsychState& agent, ConceptId concept_id) const {
    const auto before = agent.values.size();
    agent.values.erase(std::remove_if(agent.values.begin(), agent.values.end(), [&](const ValueEntry& entry) {
        return entry.concept_id == concept_id;
    }), agent.values.end());
    if (before == agent.values.size()) return make_error(ErrorCode::InvalidArgument, "value does not exist");
    return {};
}

float ValueSystem::intensity(const AgentPsychState& agent, ConceptId concept_id) const noexcept {
    const auto it = std::find_if(agent.values.begin(), agent.values.end(), [&](const ValueEntry& entry) {
        return entry.concept_id == concept_id;
    });
    return it == agent.values.end() ? 0.0f : it->intensity;
}

} // namespace mspa
