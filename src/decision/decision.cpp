#include "mspa/decision/decision.hpp"

#include <algorithm>

namespace mspa {
namespace {

float clamp01(float value) noexcept {
    return std::clamp(value, 0.0f, 1.0f);
}

float relationship_quality(const AgentPsychState& agent, AgentId target) noexcept {
    const auto it = std::find_if(agent.relationships.begin(), agent.relationships.end(),
                                 [&](const RelationshipState& relation) { return relation.other == target; });
    if (it == agent.relationships.end()) return 0.0f;
    return std::clamp((it->affection + it->trust + it->respect) / 3.0f, -1.0f, 1.0f);
}

} // namespace

CandidateScore UtilityComposer::score(const ActionOption& option) const {
    CandidateScore result{};
    result.action = option.instance;

    const float cost_term = -std::max(0.0f, option.cost);
    const float risk_term = -std::max(0.0f, option.risk);
    const float progress_term = clamp01(option.progress);

    result.considerations.push_back({"cost", option.cost, cost_term});
    result.considerations.push_back({"risk", option.risk, risk_term});
    result.considerations.push_back({"progress", option.progress, progress_term});
    result.utility = cost_term + risk_term + progress_term;
    return result;
}

CandidateScore UtilityComposer::score(const ActionOption& option,
                                      const AgentPsychState& agent,
                                      const DecisionContext& context) const {
    CandidateScore result{};
    result.action = option.instance;

    const float boldness = clamp01(agent.personality.boldness);
    const float persistence = clamp01(agent.personality.persistence);
    const float curiosity = clamp01(agent.personality.curiosity);
    const float sociability = clamp01(agent.personality.sociability);
    const float empathy = clamp01(agent.personality.empathy);

    const float cost_term = -std::max(0.0f, option.cost);
    const float risk_weight = 1.5f - boldness;
    const float risk_term = -std::max(0.0f, option.risk) * risk_weight;
    const float progress_weight = 0.5f + persistence;
    const float progress_term = clamp01(option.progress) * progress_weight;
    const float novelty_term = clamp01(option.novelty) * (curiosity - 0.5f) * 0.8f;
    const float social_term = clamp01(option.social_exposure) * (sociability - 0.5f) * 0.6f;

    result.considerations.push_back({"cost", option.cost, cost_term});
    result.considerations.push_back({"risk/personality", option.risk, risk_term});
    result.considerations.push_back({"progress/persistence", option.progress, progress_term});
    if (option.novelty != 0.0f) result.considerations.push_back({"novelty/curiosity", option.novelty, novelty_term});
    if (option.social_exposure != 0.0f) result.considerations.push_back({"social/sociability", option.social_exposure, social_term});

    result.utility = cost_term + risk_term + progress_term + novelty_term + social_term;

    for (const auto& effect : option.expected_effects) {
        float weighted = 0.0f;
        float raw = effect.magnitude;
        const char* name = "effect";

        switch (effect.channel) {
        case EffectChannel::Need: {
            name = "need";
            const auto it = std::find_if(context.needs.begin(), context.needs.end(),
                                         [&](const NeedSignal& need) { return need.concept == effect.concept; });
            if (it != context.needs.end()) {
                raw = clamp01(it->pressure);
                weighted = raw * effect.magnitude * 2.0f;
            }
            break;
        }
        case EffectChannel::Value: {
            name = "value";
            const auto it = std::find_if(agent.values.begin(), agent.values.end(),
                                         [&](const ValueEntry& value) { return value.concept_id == effect.concept; });
            if (it != agent.values.end()) {
                raw = clamp01(it->intensity);
                weighted = raw * effect.magnitude;
            }
            break;
        }
        case EffectChannel::Goal: {
            name = "goal";
            const auto it = std::find_if(agent.goals.begin(), agent.goals.end(),
                                         [&](const GoalState& goal) {
                                             return goal.active && goal.concept_id == effect.concept;
                                         });
            if (it != agent.goals.end()) {
                raw = clamp01(it->priority);
                weighted = raw * effect.magnitude * 1.25f;
            }
            break;
        }
        case EffectChannel::Relationship: {
            name = "relationship/empathy";
            raw = relationship_quality(agent, effect.target);
            weighted = raw * effect.magnitude * (0.5f + empathy);
            break;
        }
        case EffectChannel::Emotion: {
            name = "emotion";
            const auto it = std::find_if(agent.emotions.begin(), agent.emotions.end(),
                                         [&](const EmotionState& emotion) { return emotion.kind == effect.concept; });
            if (it != agent.emotions.end()) {
                raw = clamp01(it->intensity);
                weighted = raw * effect.magnitude;
            }
            break;
        }
        case EffectChannel::Generic:
            name = "generic effect";
            weighted = effect.magnitude;
            break;
        }

        result.considerations.push_back({name, raw, weighted});
        result.utility += weighted;
    }

    const auto learned = std::find_if(agent.learned_expectations.begin(), agent.learned_expectations.end(),
                                      [&](const LearnedExpectation& expectation) {
                                          return expectation.action == option.type && expectation.attempts > 0;
                                      });
    if (learned != agent.learned_expectations.end()) {
        const float success_rate = static_cast<float>(learned->successes) /
                                   static_cast<float>(learned->attempts);
        const float learned_term = (success_rate * 2.0f - 1.0f) * clamp01(learned->confidence) * 0.5f;
        result.considerations.push_back({"learned expectation", success_rate, learned_term});
        result.utility += learned_term;
    }

    return result;
}

} // namespace mspa
