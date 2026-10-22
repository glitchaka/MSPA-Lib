#include "mspa/psychology/learning.hpp"

#include <algorithm>

namespace mspa {

void LearningSystem::report(AgentPsychState& agent, ActionTypeId action, bool success) const {
    auto it = std::find_if(agent.learned_expectations.begin(), agent.learned_expectations.end(),
                           [&](const LearnedExpectation& e) { return e.action == action; });
    if (it == agent.learned_expectations.end()) {
        agent.learned_expectations.push_back({action, 1u, success ? 1u : 0u, success ? 1.0f : 0.0f});
        return;
    }
    ++it->attempts;
    if (success) ++it->successes;
    it->confidence = static_cast<float>(it->successes) / static_cast<float>(it->attempts);
}

float LearningSystem::expected_success(const AgentPsychState& agent, ActionTypeId action) const noexcept {
    auto it = std::find_if(agent.learned_expectations.begin(), agent.learned_expectations.end(),
                           [&](const LearnedExpectation& e) { return e.action == action; });
    return it == agent.learned_expectations.end() ? 0.5f : it->confidence;
}

} // namespace mspa
