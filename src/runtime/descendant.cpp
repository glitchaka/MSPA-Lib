#include "mspa/runtime/runtime.hpp"
#include "mspa/psychology/personality.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <new>
#include <set>
#include <string>
#include <vector>

namespace mspa {
namespace {
float clamp01(float value) { return std::clamp(value, 0.0f, 1.0f); }

std::vector<std::string> canonical_tags(std::vector<std::string> tags) {
    std::sort(tags.begin(), tags.end());
    tags.erase(std::unique(tags.begin(), tags.end()), tags.end());
    return tags;
}

struct LearningAggregate {
    ActionTypeId action{};
    std::vector<std::string> context_tags{};
    std::string outcome_tag{};
    std::uint64_t attempts{0};
    std::uint64_t successes{0};
    float confidence_sum{0.0f};
    std::uint32_t contributors{0};
};

bool same_pattern(const LearningAggregate& aggregate, const LearnedExpectation& expectation,
                  const std::vector<std::string>& canonical_context) {
    return aggregate.action == expectation.action &&
           aggregate.outcome_tag == expectation.outcome_tag &&
           aggregate.context_tags == canonical_context;
}

void blend_personality(const std::vector<const AgentPsychState*>& parents,
                       TraitBlendPolicy policy, Personality& output) {
    if (policy == TraitBlendPolicy::FirstSource) {
        output = parents.front()->personality;
        return;
    }
    output = {};
    output.sociability = output.stability = output.empathy = 0.0f;
    output.persistence = output.curiosity = output.boldness = 0.0f;
    for (const auto* parent : parents) {
        output.sociability += parent->personality.sociability;
        output.stability += parent->personality.stability;
        output.empathy += parent->personality.empathy;
        output.persistence += parent->personality.persistence;
        output.curiosity += parent->personality.curiosity;
        output.boldness += parent->personality.boldness;
    }
    const float n = static_cast<float>(parents.size());
    output.sociability /= n;
    output.stability /= n;
    output.empathy /= n;
    output.persistence /= n;
    output.curiosity /= n;
    output.boldness /= n;
}

void blend_values(const std::vector<const AgentPsychState*>& parents,
                  InheritedValuePolicy policy, std::vector<ValueEntry>& output) {
    if (policy != InheritedValuePolicy::Mean) return;
    std::map<ConceptId, std::pair<float, std::uint32_t>> values;
    for (const auto* parent : parents) {
        for (const auto& value : parent->values) {
            auto& aggregate = values[value.concept_id];
            aggregate.first += value.intensity;
            ++aggregate.second;
        }
    }
    for (const auto& entry : values) {
        output.push_back({entry.first, entry.second.first / static_cast<float>(entry.second.second)});
    }
}

std::vector<LearnedExpectation> inherit_learning(
    const std::vector<const AgentPsychState*>& parents,
    float evidence_retention,
    std::size_t maximum_patterns,
    PsychTime now) {
    std::vector<LearningAggregate> aggregates;
    for (const auto* parent : parents) {
        for (const auto& expectation : parent->learned_expectations) {
            if (!expectation.action.valid() || expectation.attempts == 0) continue;
            const auto context = canonical_tags(expectation.context_tags);
            auto it = std::find_if(aggregates.begin(), aggregates.end(), [&](const auto& aggregate) {
                return same_pattern(aggregate, expectation, context);
            });
            if (it == aggregates.end()) {
                aggregates.push_back({expectation.action, context, expectation.outcome_tag});
                it = std::prev(aggregates.end());
            }
            it->attempts += expectation.attempts;
            it->successes += expectation.successes;
            it->confidence_sum += clamp01(expectation.confidence);
            ++it->contributors;
        }
    }

    std::vector<LearnedExpectation> inherited;
    inherited.reserve(aggregates.size());
    for (const auto& aggregate : aggregates) {
        if (aggregate.contributors == 0) continue;
        const float contributors = static_cast<float>(aggregate.contributors);
        const float mean_attempts = static_cast<float>(aggregate.attempts) / contributors;
        const float mean_confidence = aggregate.confidence_sum / contributors;
        const auto attempts = static_cast<std::uint32_t>(std::max<long>(
            1L, std::lround(mean_attempts * evidence_retention)));
        const float target_confidence = clamp01(
            0.5f + (mean_confidence - 0.5f) * evidence_retention);
        long successes = std::lround(target_confidence * static_cast<float>(attempts + 2U) - 1.0f);
        successes = std::clamp<long>(successes, 0L, static_cast<long>(attempts));

        LearnedExpectation expectation{};
        expectation.action = aggregate.action;
        expectation.context_tags = aggregate.context_tags;
        expectation.outcome_tag = aggregate.outcome_tag;
        expectation.attempts = attempts;
        expectation.successes = static_cast<std::uint32_t>(successes);
        expectation.confidence = (static_cast<float>(expectation.successes) + 1.0f) /
                                 (static_cast<float>(expectation.attempts) + 2.0f);
        expectation.last_activation = now;
        inherited.push_back(std::move(expectation));
    }

    std::stable_sort(inherited.begin(), inherited.end(), [](const auto& a, const auto& b) {
        const float strength_a = std::fabs(a.confidence - 0.5f);
        const float strength_b = std::fabs(b.confidence - 0.5f);
        if (strength_a != strength_b) return strength_a > strength_b;
        if (a.action != b.action) return a.action < b.action;
        if (a.context_tags != b.context_tags) return a.context_tags < b.context_tags;
        return a.outcome_tag < b.outcome_tag;
    });
    if (inherited.size() > maximum_patterns) inherited.resize(maximum_patterns);
    return inherited;
}
} // namespace

Result<void> Runtime::create_agent_descendant(AgentId id, const DescendantInit& init_spec,
                                               PsychTime now) noexcept {
    try {
        if (init_spec.parent_agents.empty())
            return make_error(ErrorCode::InvalidArgument, "descendant creation requires parent agents");
        if (!std::isfinite(init_spec.variation_strength) || init_spec.variation_strength < 0.0f ||
            init_spec.variation_strength > 1.0f)
            return make_error(ErrorCode::InvalidArgument, "descendant variation strength must be in [0,1]");
        if (!std::isfinite(init_spec.inherited_learning_evidence) ||
            init_spec.inherited_learning_evidence < 0.0f || init_spec.inherited_learning_evidence > 1.0f)
            return make_error(ErrorCode::InvalidArgument, "inherited learning evidence must be in [0,1]");

        std::set<AgentId> unique_parents;
        std::vector<const AgentPsychState*> parents;
        parents.reserve(init_spec.parent_agents.size());
        for (const auto parent_id : init_spec.parent_agents) {
            if (!unique_parents.insert(parent_id).second)
                return make_error(ErrorCode::InvalidArgument, "duplicate descendant parent");
            const auto* parent = agents().find(parent_id);
            if (!parent) return make_error(ErrorCode::UnknownAgent, "descendant parent missing");
            parents.push_back(parent);
        }

        AgentInit child{};
        blend_personality(parents, init_spec.trait_blend_policy, child.personality);
        blend_values(parents, init_spec.inherited_value_policy, child.values);

        if (init_spec.inherited_learning_policy == InheritedLearningPolicy::Mean &&
            init_spec.inherited_learning_evidence > 0.0f) {
            const auto runtime_limit = config().limits.max_learning_patterns_per_agent;
            const auto requested_limit = init_spec.max_inherited_learning_patterns == 0
                ? runtime_limit
                : std::min(runtime_limit, init_spec.max_inherited_learning_patterns);
            child.learned_expectations = inherit_learning(
                parents, init_spec.inherited_learning_evidence, requested_limit, now);
        }

        if (init_spec.variation_strength > 0.0f) {
            DeterministicRng rng(derive_agent_seed(init_spec.variation_seed, id, 0x44455343454e44ULL));
            auto vary = [&](float& value) {
                value = clamp01(value +
                    (rng.next_unit_float() * 2.0f - 1.0f) * init_spec.variation_strength);
            };
            vary(child.personality.sociability);
            vary(child.personality.stability);
            vary(child.personality.empathy);
            vary(child.personality.persistence);
            vary(child.personality.curiosity);
            vary(child.personality.boldness);
        }

        // Beliefs, autobiographical memories and relationships are intentionally not inherited.
        // What crosses generations is a weakened prior about action outcomes, not personal history.
        return create_agent(id, child, now);
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "descendant creation allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation,
                          "unexpected descendant creation exception");
    }
}

} // namespace mspa
