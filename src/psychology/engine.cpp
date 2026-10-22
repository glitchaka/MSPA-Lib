#include "mspa/psychology/engine.hpp"

#include <algorithm>

namespace mspa {

Result<bool> PsychologyEngine::observe(
    AgentPsychState& agent,
    const Proposition& proposition,
    PsychTime process_time,
    PsychTime event_time,
    bool has_event_time,
    const RegistrySnapshot& registry,
    AppraisalResult* appraisal_out) const {

    if (!agent.id.valid() || !proposition.predicate.valid()) {
        return make_error(ErrorCode::InvalidArgument, "agent and predicate must be valid");
    }
    if (process_time < agent.last_processed_time) {
        return make_error(ErrorCode::TimeRegression, "process_time regressed");
    }

    const auto* schema = registry.find(proposition.predicate);
    AppraisalResult appraisal{};
    appraisal.relevance = schema ? 1.0f : 0.25f;
    appraisal.consequence = proposition.positive ? 0.25f : -0.25f;
    appraisal.control = 0.5f;
    appraisal.norm_alignment = 0.0f;

    BeliefState belief{};
    belief.id = BeliefId(static_cast<std::uint64_t>(agent.beliefs.size() + 1));
    belief.predicate = proposition.predicate;
    belief.certainty = schema ? 0.75f : 0.4f;
    belief.emotional_load = appraisal.consequence * appraisal.relevance;
    belief.updated_at = has_event_time ? event_time : process_time;
    agent.beliefs.push_back(belief);
    agent.last_processed_time = process_time;

    if (appraisal_out) {
        *appraisal_out = appraisal;
    }
    return true;
}

} // namespace mspa
