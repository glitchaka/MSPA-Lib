#include "mspa/semantics/registry.hpp"

namespace mspa {

RegistrySnapshot::RegistrySnapshot() : data_(std::make_shared<RegistryData>()) {}

RegistrySnapshot::RegistrySnapshot(std::shared_ptr<const RegistryData> data)
    : data_(data ? std::move(data) : std::make_shared<RegistryData>()) {}

const RegistryData& RegistrySnapshot::data() const noexcept {
    return *data_;
}

const PredicateSchema* RegistrySnapshot::find(PredicateId id) const noexcept {
    const auto it = data_->predicates.find(id);
    return it == data_->predicates.end() ? nullptr : &it->second;
}

RegistryBuilder::RegistryBuilder() = default;

RegistryBuilder::RegistryBuilder(const RegistrySnapshot& base) : data_(base.data()) {}

Result<void> RegistryBuilder::add_predicate(PredicateSchema schema) {
    if (!schema.id.valid() || !schema.name_space.valid()) {
        return make_error(ErrorCode::InvalidArgument, "predicate id and namespace must be valid");
    }
    const auto [it, inserted] = data_.predicates.emplace(schema.id, std::move(schema));
    if (!inserted) {
        return make_error(ErrorCode::NamespaceConflict, "predicate already registered");
    }
    return {};
}

Result<void> RegistryBuilder::add_concept(ConceptId id, std::string name) {
    if (!id.valid() || name.empty()) {
        return make_error(ErrorCode::InvalidArgument, "concept id and name are required");
    }
    const auto [it, inserted] = data_.concepts.emplace(id, std::move(name));
    if (!inserted) {
        return make_error(ErrorCode::NamespaceConflict, "concept already registered");
    }
    return {};
}

Result<void> RegistryBuilder::add_action_type(ActionTypeId id, std::string name) {
    if (!id.valid() || name.empty()) {
        return make_error(ErrorCode::InvalidArgument, "action type id and name are required");
    }
    const auto [it, inserted] = data_.action_types.emplace(id, std::move(name));
    if (!inserted) {
        return make_error(ErrorCode::NamespaceConflict, "action type already registered");
    }
    return {};
}

RegistrySnapshot RegistryBuilder::publish() const {
    return RegistrySnapshot(std::make_shared<const RegistryData>(data_));
}

} // namespace mspa
