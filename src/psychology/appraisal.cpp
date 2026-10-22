#include "mspa/psychology/appraisal.hpp"

namespace mspa {

AppraisalVector AppraisalEngine::evaluate(const AgentPsychState& agent,
                                          const Proposition& proposition,
                                          const RegistrySnapshot& registry) const {
    AppraisalVector result{};
    const auto* schema = registry.find(proposition.predicate);
    result.relevance = schema ? 1.0f : 0.25f;
    result.consequence = proposition.positive ? 0.25f : -0.25f;
    result.agency = schema && !schema->agency_role.empty() ? 1.0f : 0.0f;
    result.control = agent.personality.boldness * 0.5f + agent.personality.persistence * 0.5f;
    result.norm_alignment = 0.0f;
    result.expectation_error = 0.0f;
    return result;
}

} // namespace mspa
