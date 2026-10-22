#include "mspa/decision/decision.hpp"

#include <algorithm>

namespace mspa {

CandidateScore UtilityComposer::score(const ActionOption& option) const {
    CandidateScore result{};
    result.action = option.instance;

    const float cost_term = -std::max(0.0f, option.cost);
    const float risk_term = -std::max(0.0f, option.risk);
    const float progress_term = std::clamp(option.progress, 0.0f, 1.0f);

    result.considerations.push_back({"cost", option.cost, cost_term});
    result.considerations.push_back({"risk", option.risk, risk_term});
    result.considerations.push_back({"progress", option.progress, progress_term});
    result.utility = cost_term + risk_term + progress_term;
    return result;
}

} // namespace mspa
