#include "mspa/semantics/domain_pack.hpp"

namespace mspa {

Result<void> validate_domain_pack(const DomainPack& pack) {
    if (!pack.metadata.id.valid() || !pack.metadata.name_space.valid()) {
        return make_error(ErrorCode::InvalidPack, "domain pack requires valid pack and namespace ids");
    }
    for (const auto& schema : pack.predicates) {
        if (!schema.id.valid() || !schema.name_space.valid()) {
            return make_error(ErrorCode::InvalidPack, "domain pack contains invalid predicate schema");
        }
    }
    return {};
}

Result<void> apply_domain_pack(RegistryBuilder& builder, const DomainPack& pack) {
    auto valid = validate_domain_pack(pack);
    if (!valid) return *valid.error_if();

    for (const auto& schema : pack.predicates) {
        auto result = builder.add_predicate(schema);
        if (!result) return *result.error_if();
    }
    for (const auto& entry : pack.concepts) {
        auto result = builder.add_concept(entry.first, entry.second);
        if (!result) return *result.error_if();
    }
    for (const auto& entry : pack.action_types) {
        auto result = builder.add_action_type(entry.first, entry.second);
        if (!result) return *result.error_if();
    }
    return {};
}

} // namespace mspa
