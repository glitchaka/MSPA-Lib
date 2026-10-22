#include "mspa/runtime/agent_store.hpp"

namespace mspa {

Result<void> AgentStore::insert(AgentPsychState state) {
    if (!state.id.valid()) {
        return make_error(ErrorCode::InvalidArgument, "agent id must be valid");
    }
    const auto [it, inserted] = agents_.emplace(state.id, std::move(state));
    if (!inserted) {
        return make_error(ErrorCode::DuplicateAgent, "agent already exists");
    }
    return {};
}

Result<void> AgentStore::erase(AgentId id) {
    if (!id.valid()) {
        return make_error(ErrorCode::InvalidArgument, "agent id must be valid");
    }
    if (agents_.erase(id) == 0) {
        return make_error(ErrorCode::UnknownAgent, "agent does not exist");
    }
    return {};
}

bool AgentStore::contains(AgentId id) const noexcept {
    return agents_.find(id) != agents_.end();
}

AgentPsychState* AgentStore::find(AgentId id) noexcept {
    const auto it = agents_.find(id);
    return it == agents_.end() ? nullptr : &it->second;
}

const AgentPsychState* AgentStore::find(AgentId id) const noexcept {
    const auto it = agents_.find(id);
    return it == agents_.end() ? nullptr : &it->second;
}

std::size_t AgentStore::size() const noexcept {
    return agents_.size();
}

} // namespace mspa
