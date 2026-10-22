#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "mspa/base/ids.hpp"
#include "mspa/base/result.hpp"

namespace mspa {

struct PropositionArg {
    std::string role{};
    std::string value{};
};

struct Proposition {
    PredicateId predicate{};
    std::vector<PropositionArg> arguments{};
    bool positive{true};
    NamespaceId name_space{};
};

struct PredicateSchema {
    PredicateId id{};
    NamespaceId name_space{};
    std::vector<std::string> required_roles{};
    std::string agency_role{};
    std::vector<std::string> consequence_tags{};
    std::vector<std::string> appraisal_hints{};
};

struct RegistryData {
    std::map<PredicateId, PredicateSchema> predicates{};
    std::map<ConceptId, std::string> concepts{};
    std::map<ActionTypeId, std::string> action_types{};
};

class RegistrySnapshot {
public:
    RegistrySnapshot();
    explicit RegistrySnapshot(std::shared_ptr<const RegistryData> data);

    [[nodiscard]] const RegistryData& data() const noexcept;
    [[nodiscard]] const PredicateSchema* find(PredicateId id) const noexcept;

private:
    std::shared_ptr<const RegistryData> data_;
};

class RegistryBuilder {
public:
    RegistryBuilder();
    explicit RegistryBuilder(const RegistrySnapshot& base);

    Result<void> add_predicate(PredicateSchema schema);
    Result<void> add_concept(ConceptId id, std::string name);
    Result<void> add_action_type(ActionTypeId id, std::string name);
    [[nodiscard]] RegistrySnapshot publish() const;

private:
    RegistryData data_;
};

} // namespace mspa
