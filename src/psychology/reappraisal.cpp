#include "mspa/psychology/reappraisal.hpp"

#include <algorithm>

namespace mspa {

Result<void> ReappraisalSystem::reappraise_memory(AgentPsychState& agent, MemoryId id,
                                                   float emotional_load, float certainty) const {
    auto it = std::find_if(agent.memories.begin(), agent.memories.end(), [&](const MemoryState& memory) {
        return memory.id == id;
    });
    if (it == agent.memories.end()) return make_error(ErrorCode::InvalidArgument, "memory does not exist");
    it->emotional_load = std::clamp(emotional_load, -1.0f, 1.0f);
    it->certainty = std::clamp(certainty, 0.0f, 1.0f);
    return {};
}

Result<void> ReappraisalSystem::reactivate_memory(AgentPsychState& agent, MemoryId id) const {
    auto it = std::find_if(agent.memories.begin(), agent.memories.end(), [&](const MemoryState& memory) {
        return memory.id == id;
    });
    if (it == agent.memories.end()) return make_error(ErrorCode::InvalidArgument, "memory does not exist");
    ++it->reactivations;
    return {};
}

} // namespace mspa
