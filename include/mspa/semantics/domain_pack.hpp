#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "mspa/base/result.hpp"
#include "mspa/semantics/inference.hpp"
#include "mspa/semantics/registry.hpp"

namespace mspa {

struct DomainPackMetadata {
    PackId id{};
    NamespaceId name_space{};
    std::uint32_t version_major{0};
    std::uint32_t version_minor{1};
    std::uint32_t version_patch{0};
    std::string name{};
};

struct DomainPack {
    DomainPackMetadata metadata{};
    std::vector<PredicateSchema> predicates{};
    std::vector<std::pair<ConceptId, std::string>> concepts{};
    std::vector<std::pair<ActionTypeId, std::string>> action_types{};
    std::vector<InferenceRule> inference_rules{};
};

Result<void> validate_domain_pack(const DomainPack& pack);
Result<void> apply_domain_pack(RegistryBuilder& builder, const DomainPack& pack);

} // namespace mspa
