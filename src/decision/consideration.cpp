#include "mspa/decision/consideration.hpp"

namespace mspa {

Result<void> ConsiderationRegistry::add(ConsiderationDefinition definition) {
    if (definition.id.empty()) return make_error(ErrorCode::InvalidArgument, "consideration id cannot be empty");
    if (definitions_.find(definition.id) != definitions_.end()) {
        return make_error(ErrorCode::InvalidArgument, "consideration id already exists");
    }
    definitions_.emplace(definition.id, std::move(definition));
    return {};
}

Result<void> ConsiderationRegistry::remove(const std::string& id) {
    if (definitions_.erase(id) == 0) return make_error(ErrorCode::InvalidArgument, "consideration does not exist");
    return {};
}

void ConsiderationRegistry::clear() noexcept { definitions_.clear(); }

const ConsiderationDefinition* ConsiderationRegistry::find(const std::string& id) const noexcept {
    const auto it = definitions_.find(id);
    return it == definitions_.end() ? nullptr : &it->second;
}

std::size_t ConsiderationRegistry::size() const noexcept { return definitions_.size(); }

} // namespace mspa
