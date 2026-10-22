#include <cassert>
#include <memory>
#include <vector>

#include "mspa/mspa.hpp"

using namespace mspa;

int main() {
    RegistryBuilder registry_builder;
    PredicateSchema schema{};
    schema.id = PredicateId(1);
    schema.name_space = NamespaceId(1);
    schema.required_roles = {"actor"};
    assert(registry_builder.add_predicate(schema));
    auto registry = registry_builder.publish();

    RuntimeConfig config{};
    config.runtime_id = RuntimeId(1);
    config.world_id = WorldId(10);
    config.time_domain = TimeDomain{TimeDomainId(1), TimeScale{60, 1}};
    config.seed_root = 12345;

    auto runtime_result = Runtime::create(config, registry);
    assert(runtime_result);
    auto runtime = std::move(*runtime_result.value_if());

    AgentId agent{config.world_id, 1};
    AgentInit init{};
    init.personality.curiosity = 0.8f;
    assert(runtime->create_agent(agent, init, PsychTime(10)));
    assert(runtime->has_agent(agent));

    ObserveInput observation{};
    observation.agent = agent;
    observation.proposition.predicate = PredicateId(1);
    observation.proposition.name_space = NamespaceId(1);
    observation.time.process_time = PsychTime(12);
    observation.time.has_event_time = true;
    observation.time.event_time = PsychTime(5);

    auto observed = runtime->observe(observation);
    assert(observed);
    assert(observed.value_if()->state_changed);
    assert(runtime->agents().find(agent)->beliefs.size() == 1);

    ActionOption a{};
    a.type = ActionTypeId(1);
    a.instance = ActionInstanceId(1);
    a.continuity = ActionContinuityKey(1);
    a.actor = agent;
    a.cost = 0.1f;
    a.risk = 0.2f;
    a.progress = 0.9f;

    ActionOption b{};
    b.type = ActionTypeId(2);
    b.instance = ActionInstanceId(2);
    b.continuity = ActionContinuityKey(2);
    b.actor = agent;
    b.cost = 0.8f;
    b.risk = 0.8f;
    b.progress = 0.1f;

    auto decision = runtime->decide(agent, std::vector<ActionOption>{a, b}, PsychTime(13));
    assert(decision);
    assert(decision.value_if()->has_selection);
    assert(decision.value_if()->selected_action == a.instance);

    ReportInput report{};
    report.agent = agent;
    report.action_type = a.type;
    report.success = true;
    report.process_time = PsychTime(14);
    assert(runtime->report(report));
    assert(runtime->agents().find(agent)->learned_expectations.size() == 1);

    auto exported = runtime->export_agent(agent, PsychTime(20));
    assert(exported);

    RuntimeConfig destination_config{};
    destination_config.runtime_id = RuntimeId(2);
    destination_config.world_id = WorldId(20);
    destination_config.time_domain = TimeDomain{TimeDomainId(2), TimeScale{120, 1}};
    destination_config.seed_root = 99999;

    auto destination_result = Runtime::create(destination_config, registry);
    assert(destination_result);
    auto destination = std::move(*destination_result.value_if());

    ImportOptions import_options{};
    import_options.destination_id = AgentId{destination_config.world_id, 1};
    import_options.import_time = PsychTime(100);
    import_options.time_policy = ImportTimePolicy::RebaseRelative;
    import_options.rng_policy = RngImportPolicy::RekeyForDestination;
    import_options.destination_seed_root = destination_config.seed_root;

    auto imported = destination->import_agent(*exported.value_if(), import_options);
    assert(imported);
    assert(imported.value_if()->original_id == agent);
    assert(imported.value_if()->effective_id == import_options.destination_id);
    assert(destination->has_agent(import_options.destination_id));

    assert(runtime->diagnostics().traces().size() >= 2);
    assert(runtime->remove_agent(agent, PsychTime(21)));
    assert(!runtime->has_agent(agent));

    return 0;
}
