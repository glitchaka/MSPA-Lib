#include "mspa/semantics/inference.hpp"

#include <algorithm>

namespace mspa {

Result<void> InferenceEngine::add_rule(InferenceRule rule) {
    if (rule.id == 0 || !rule.conclusion.valid()) {
        return make_error(ErrorCode::InvalidArgument, "inference rule requires id and conclusion");
    }
    if (std::any_of(rules_.begin(), rules_.end(), [&](const InferenceRule& existing) { return existing.id == rule.id; })) {
        return make_error(ErrorCode::InvalidArgument, "duplicate inference rule id");
    }
    rules_.push_back(std::move(rule));
    return {};
}

void InferenceEngine::clear_rules() noexcept { rules_.clear(); }

std::size_t InferenceEngine::rule_count() const noexcept { return rules_.size(); }

const std::vector<InferenceRule>& InferenceEngine::rules() const noexcept { return rules_; }

Result<std::vector<Hypothesis>> InferenceEngine::infer(const std::vector<Proposition>& facts,
                                                       const RegistrySnapshot&,
                                                       const InferenceBudget& budget) const {
    if (budget.max_depth == 0 || budget.max_rules == 0 || budget.max_hypotheses == 0) {
        return std::vector<Hypothesis>{};
    }

    std::vector<Hypothesis> output;
    const std::size_t rule_limit = std::min(rules_.size(), budget.max_rules);
    for (std::size_t i = 0; i < rule_limit && output.size() < budget.max_hypotheses; ++i) {
        const auto& rule = rules_[i];
        const bool matched = std::all_of(rule.premises.begin(), rule.premises.end(), [&](PredicateId premise) {
            return std::any_of(facts.begin(), facts.end(), [&](const Proposition& fact) {
                return fact.predicate == premise && fact.positive;
            });
        });
        if (!matched) continue;

        Hypothesis hypothesis{};
        hypothesis.proposition.predicate = rule.conclusion;
        hypothesis.proposition.name_space = rule.conclusion_namespace;
        hypothesis.proposition.positive = true;
        hypothesis.certainty = std::clamp(rule.base_certainty, 0.0f, 1.0f);
        output.push_back(std::move(hypothesis));
    }
    return output;
}

} // namespace mspa
