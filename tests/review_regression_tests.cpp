#include <cstdlib>
#include <iostream>
#include <vector>

#include "mspa/mspa.hpp"

using namespace mspa;

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

void test_psychology_changes_decision_scores() {
    UtilityComposer utility;
    const AgentId actor{WorldId{1}, 1};

    ActionOption risky{};
    risky.type = ActionTypeId{1};
    risky.instance = ActionInstanceId{1};
    risky.actor = actor;
    risky.risk = 1.0f;
    risky.progress = 0.8f;

    ActionOption safe{};
    safe.type = ActionTypeId{2};
    safe.instance = ActionInstanceId{2};
    safe.actor = actor;
    safe.risk = 0.1f;
    safe.progress = 0.3f;

    AgentPsychState cautious{};
    cautious.id = actor;
    cautious.personality.boldness = 0.0f;
    cautious.personality.persistence = 0.5f;

    AgentPsychState bold = cautious;
    bold.personality.boldness = 1.0f;

    require(utility.score(risky, cautious).utility < utility.score(safe, cautious).utility,
            "cautious personality should prefer safer option");
    require(utility.score(risky, bold).utility > utility.score(safe, bold).utility,
            "bold personality should tolerate risky progress");

    const ConceptId hunger{900};
    DecisionContext context{};
    context.needs.push_back({hunger, 1.0f});

    ActionOption eat = safe;
    eat.instance = ActionInstanceId{3};
    eat.expected_effects.push_back({EffectChannel::Need, hunger, {}, 1.0f});

    ActionOption idle = safe;
    idle.instance = ActionInstanceId{4};

    require(utility.score(eat, cautious, context).utility > utility.score(idle, cautious, context).utility,
            "host-owned need pressure should affect utility");

    const AgentId friend_id{WorldId{1}, 2};
    cautious.relationships.push_back({friend_id, 1.0f, 1.0f, 1.0f, 1.0f});

    ActionOption help = safe;
    help.instance = ActionInstanceId{5};
    help.expected_effects.push_back({EffectChannel::Relationship, {}, friend_id, 1.0f});

    ActionOption harm = safe;
    harm.instance = ActionInstanceId{6};
    harm.expected_effects.push_back({EffectChannel::Relationship, {}, friend_id, -1.0f});

    require(utility.score(help, cautious).utility > utility.score(harm, cautious).utility,
            "relationships and empathy should affect utility");
}

void test_observation_validation_and_belief_quota() {
    RegistryBuilder builder;
    PredicateSchema schema{};
    schema.id = PredicateId{1};
    schema.name_space = NamespaceId{1};
    schema.required_roles = {"actor"};
    require(static_cast<bool>(builder.add_predicate(schema)), "register schema");

    RuntimeConfig config{};
    config.runtime_id = RuntimeId{1};
    config.world_id = WorldId{10};
    config.time_domain = TimeDomain{TimeDomainId{1}, TimeScale{1, 1}};
    config.limits.max_beliefs_per_agent = 1;

    auto runtime_result = Runtime::create(config, builder.publish());
    require(static_cast<bool>(runtime_result), "create runtime");
    auto runtime = std::move(*runtime_result.value_if());

    const AgentId agent{config.world_id, 1};
    require(static_cast<bool>(runtime->create_agent(agent, AgentInit{}, PsychTime{0})), "create agent");

    ObserveInput incomplete{};
    incomplete.agent = agent;
    incomplete.proposition.predicate = PredicateId{1};
    incomplete.proposition.name_space = NamespaceId{1};
    incomplete.time.process_time = PsychTime{1};

    auto invalid = runtime->observe(incomplete);
    require(!invalid && invalid.error_if()->code == ErrorCode::InvalidArgument,
            "missing required role must be rejected");
    require(runtime->agents().find(agent)->beliefs.empty(),
            "rejected observation must not mutate beliefs");

    ObserveInput valid = incomplete;
    valid.proposition.arguments.push_back({"actor", "agent:1"});
    auto first = runtime->observe(valid);
    require(static_cast<bool>(first), "first valid observation should succeed");
    require(runtime->agents().find(agent)->beliefs.size() == 1,
            "first observation should fill belief quota");

    valid.time.process_time = PsychTime{2};
    auto overflow = runtime->observe(valid);
    require(!overflow && overflow.error_if()->code == ErrorCode::QuotaExceeded,
            "belief quota must be enforced before mutation");
    require(runtime->agents().find(agent)->beliefs.size() == 1,
            "quota failure must leave state unchanged");
}

void test_import_rebases_every_psychological_timestamp() {
    ExportedAgent exported{};
    exported.state.id = AgentId{WorldId{1}, 1};
    exported.state.last_processed_time = PsychTime{20};
    exported.state.beliefs.push_back({BeliefId{1}, PredicateId{1}, 1.0f, 0.0f, PsychTime{12}});
    exported.state.memories.push_back({MemoryId{1}, PredicateId{1}, 1.0f, 0.0f, 1.0f, 0, PsychTime{5}});
    exported.state.emotions.push_back({ConceptId{7}, 1.0f, PsychTime{10}});
    exported.state.mood.updated_at = PsychTime{15};
    exported.origin_time_domain = TimeDomain{TimeDomainId{1}, TimeScale{1, 1}};
    exported.exported_at = PsychTime{20};

    ImportOptions options{};
    options.destination_id = AgentId{WorldId{2}, 1};
    options.import_time = PsychTime{100};
    options.time_policy = ImportTimePolicy::RebaseRelative;
    options.rng_policy = RngImportPolicy::RekeyForDestination;

    auto imported = import_agent_state(exported,
                                       TimeDomain{TimeDomainId{2}, TimeScale{2, 1}},
                                       options);
    require(static_cast<bool>(imported), "import with time conversion should succeed");
    const auto& state = imported.value_if()->state;

    require(state.last_processed_time == PsychTime{100}, "last_processed_time should rebase");
    require(state.beliefs.front().updated_at == PsychTime{84}, "belief time should rebase");
    require(state.memories.front().event_time == PsychTime{70}, "memory time should rebase");
    require(state.emotions.front().started_at == PsychTime{80}, "emotion time should rebase");
    require(state.mood.updated_at == PsychTime{90}, "mood time should rebase");
}

} // namespace

int main() {
    test_psychology_changes_decision_scores();
    test_observation_validation_and_belief_quota();
    test_import_rebases_every_psychological_timestamp();
    std::cout << "MSPA integration review regressions passed\n";
    return 0;
}
