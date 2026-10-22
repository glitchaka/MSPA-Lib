#include "mspa/psychology/memory.hpp"

#include <algorithm>

namespace mspa {

Result<MemoryId> MemorySystem::remember(AgentPsychState& agent, PredicateId predicate, float importance,
                                        PsychTime event_time) const {
    if (!predicate.valid()) return make_error(ErrorCode::InvalidArgument, "memory predicate must be valid");
    std::uint64_t next = 1;
    for (const auto& memory : agent.memories) next = std::max(next, memory.id.value() + 1);
    MemoryState state{};
    state.id = MemoryId(next);
    state.predicate = predicate;
    state.importance = std::clamp(importance, 0.0f, 1.0f);
    state.event_time = event_time;
    agent.memories.push_back(state);
    return state.id;
}

void MemorySystem::compact(AgentPsychState& agent, std::size_t max_memories) const {
    if (agent.memories.size() <= max_memories) return;
    std::stable_sort(agent.memories.begin(), agent.memories.end(),
                     [](const MemoryState& a, const MemoryState& b) { return a.importance > b.importance; });
    agent.memories.resize(max_memories);
}

} // namespace mspa
