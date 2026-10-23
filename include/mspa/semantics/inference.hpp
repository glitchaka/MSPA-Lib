#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "mspa/base/result.hpp"
#include "mspa/semantics/registry.hpp"

namespace mspa {

struct InferenceRule {
    std::uint64_t id{0};
    std::vector<PredicateId> premises{};
    PredicateId conclusion{};
    NamespaceId conclusion_namespace{};
    float base_certainty{0.5f};
};

struct InferenceBudget {
    std::size_t max_rules{256};
    std::size_t max_hypotheses{64};
    std::uint32_t max_depth{4};
};

struct Hypothesis {
    Proposition proposition{};
    float certainty{0.0f};
    std::uint32_t depth{1};
};

class InferenceEngine {
public:
    Result<void> add_rule(InferenceRule rule);
    void clear_rules() noexcept;
    [[nodiscard]] std::size_t rule_count() const noexcept;
    [[nodiscard]] const std::vector<InferenceRule>& rules() const noexcept;

    Result<std::vector<Hypothesis>> infer(const std::vector<Proposition>& facts,
                                          const RegistrySnapshot& registry,
                                          const InferenceBudget& budget = {}) const;

private:
    std::vector<InferenceRule> rules_{};
};

} // namespace mspa
