#include "mspa/psychology/engine.hpp"

#include <algorithm>
#include <set>

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
    if (schema) {
        if (proposition.name_space != schema->name_space) {
            return make_error(ErrorCode::InvalidArgument, "proposition namespace does not match predicate schema");
        }

        std::set<std::string> roles;
        for (const auto& argument : proposition.arguments) {
            if (argument.role.empty() || argument.value.empty()) {
                return make_error(ErrorCode::InvalidArgument, "proposition arguments require non-empty role and value");
            }
            if (!roles.insert(argument.role).second) {
                return make_error(ErrorCode::InvalidArgument, "proposition contains a duplicate role");
            }
        }
        for (const auto& required : schema->required_roles) {
            if (roles.find(required) == roles.end()) {
                return make_error(ErrorCode::InvalidArgument, "proposition is missing a required role");
            }
        }
    }

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
