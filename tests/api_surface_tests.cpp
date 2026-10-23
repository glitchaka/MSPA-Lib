#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

#include "mspa/mspa.hpp"

using namespace mspa;

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}
}

int main() {
    auto converted = convert_time_delta(PsychTime{10}, TimeScale{1, 1}, TimeScale{2, 1});
    require(static_cast<bool>(converted), "time conversion");

    const AgentId base_agent{WorldId{1}, 1};
    const auto seed = derive_agent_seed(123, base_agent, 0);
    DeterministicRng rng(seed);
    (void)rng.next_u32();
    (void)rng.next_u64();
    const auto saved_rng = rng.state();
    DeterministicRng restored(saved_rng);
    (void)restored.next_u64();

    RegistryBuilder registry_builder;
    PredicateSchema schema{};
    schema.id = PredicateId{1};
    schema.name_space = NamespaceId{1};
    schema.required_roles = {"actor"};
    require(static_cast<bool>(registry_builder.add_predicate(schema)), "registry add predicate");
    require(static_cast<bool>(registry_builder.add_concept(ConceptId{1}, "concept")), "registry add concept");
    require(static_cast<bool>(registry_builder.add_action_type(ActionTypeId{1}, "action")), "registry add action");
    const RegistrySnapshot registry = registry_builder.publish();
    require(registry.find(PredicateId{1}) != nullptr, "registry find");
    (void)registry.data();
    RegistryBuilder registry_copy(registry);
    (void)registry_copy.publish();

    PersonalitySystem personality_system;
    Personality personality{};
    require(static_cast<bool>(personality_system.normalize(personality)), "personality normalize");
    (void)personality_system.derive(personality);

    AgentPsychState state{};
    state.id = base_agent;

    ValueSystem values;
    require(static_cast<bool>(values.set(state, ConceptId{1}, 0.7f)), "value set");
    (void)values.intensity(state, ConceptId{1});
    require(static_cast<bool>(values.erase(state, ConceptId{1})), "value erase");

    BeliefSystem beliefs;
    auto belief_id = beliefs.add(state, PredicateId{1}, 0.5f, 0.1f, PsychTime{1});
    require(static_cast<bool>(belief_id), "belief add");
    require(static_cast<bool>(beliefs.reinforce(state, *belief_id.value_if(), 0.2f)), "belief reinforce");

    Proposition proposition{};
    proposition.predicate = PredicateId{1};
    proposition.name_space = NamespaceId{1};
    proposition.arguments.push_back({"actor", "entity"});

    AppraisalEngine appraisal;
    const auto appraisal_vector = appraisal.evaluate(state, proposition, registry);

    EmotionSystem emotions;
    emotions.apply(state, appraisal_vector, PsychTime{2});
    emotions.decay(state, PsychTime{3});

    MoodSystem mood;
    mood.update(state, 0.1f, 0.1f, PsychTime{3});
    mood.decay(state, PsychTime{4});
    mood.reset(state, PsychTime{5});

    GoalSystem goals;
    auto goal_id = goals.add(state, ConceptId{2}, 0.8f);
    require(static_cast<bool>(goal_id), "goal add");
    require(static_cast<bool>(goals.complete(state, *goal_id.value_if())), "goal complete");

    MemorySystem memory;
    auto memory_id = memory.remember(state, PredicateId{1}, 0.5f, PsychTime{1});
    require(static_cast<bool>(memory_id), "memory remember");
    memory.compact(state, 100);

    ReappraisalSystem reappraisal;
    require(static_cast<bool>(reappraisal.reappraise_memory(state, *memory_id.value_if(), 0.2f, 0.9f)), "reappraise");
    require(static_cast<bool>(reappraisal.reactivate_memory(state, *memory_id.value_if())), "reactivate");

    RelationshipSystem relationships;
    const AgentId other{WorldId{1}, 2};
    (void)relationships.get_or_create(state, other);
    require(relationships.find(state, other) != nullptr, "relationship find");

    LearningSystem learning;
    learning.report(state, ActionTypeId{1}, true);
    (void)learning.expected_success(state, ActionTypeId{1});

    PsychologyEngine psychology;
    AgentPsychState observed_state{};
    observed_state.id = AgentId{WorldId{1}, 3};
    AppraisalResult appraisal_result{};
    auto psych_observed = psychology.observe(observed_state, proposition, PsychTime{1}, PsychTime{1}, false,
                                             registry, &appraisal_result);
    require(static_cast<bool>(psych_observed), "psychology observe");

    InferenceEngine inference;
    InferenceRule rule{};
    rule.id = 1;
    rule.conclusion = PredicateId{2};
    rule.conclusion_namespace = NamespaceId{1};
    require(static_cast<bool>(inference.add_rule(rule)), "inference add rule");
    require(inference.rule_count() == 1 && inference.rules().size() == 1, "inference accessors");
    auto hypotheses = inference.infer({}, registry);
    require(static_cast<bool>(hypotheses), "inference run");
    inference.clear_rules();

    ResponseCurve curve;
    require(static_cast<bool>(curve.set_points({{0.0f, 0.0f}, {1.0f, 1.0f}})), "curve set");
    (void)curve.evaluate(0.5f);
    require(curve.size() == 2 && curve.points().size() == 2, "curve accessors");
    curve.clear();

    ConsiderationRegistry considerations;
    ConsiderationDefinition definition{};
    definition.id = "test";
    require(static_cast<bool>(considerations.add(std::move(definition))), "consideration add");
    require(considerations.find("test") != nullptr && considerations.size() == 1, "consideration accessors");
    require(static_cast<bool>(considerations.remove("test")), "consideration remove");
    considerations.clear();

    ActionOption option{};
    option.type = ActionTypeId{1};
    option.instance = ActionInstanceId{1};
    option.continuity = ActionContinuityKey{1};
    option.actor = base_agent;
    option.cost = 0.1f;
    option.risk = 0.2f;
    option.progress = 0.3f;
    UtilityComposer utility;
    (void)utility.score(option);
    HysteresisPolicy hysteresis;
    (void)hysteresis.continuity_bonus(state, option, option.continuity);

    DomainPack pack{};
    pack.metadata.id = PackId{1};
    pack.metadata.name_space = NamespaceId{1};
    pack.metadata.name = "neutral";
    require(static_cast<bool>(validate_domain_pack(pack)), "pack validate");
    RegistryBuilder pack_builder;
    require(static_cast<bool>(apply_domain_pack(pack_builder, pack)), "pack apply");

    RuntimeLimits limits{};
    require(static_cast<bool>(validate_limits(limits)), "validate limits");
    require(static_cast<bool>(validate_agent_state(state, limits)), "validate state");

    AgentStore store;
    AgentPsychState stored{};
    stored.id = AgentId{WorldId{8}, 1};
    require(static_cast<bool>(store.insert(stored)), "agent store insert");
    require(store.contains(stored.id) && store.find(stored.id) != nullptr && store.size() == 1, "agent store accessors");
    const AgentStore& const_store = store;
    require(const_store.find(stored.id) != nullptr, "agent store const find");
    require(static_cast<bool>(store.erase(stored.id)), "agent store erase");

    TraceSink sink;
    DiagnosticTrace trace{};
    trace.agent = base_agent;
    sink.push(trace);
    require(sink.traces().size() == 1, "diagnostics traces");
    sink.clear();

    ExportedAgent exported{};
    exported.state = state;
    exported.state.id = AgentId{WorldId{5}, 1};
    exported.origin_time_domain = TimeDomain{TimeDomainId{5}, TimeScale{1, 1}};
    exported.exported_at = PsychTime{10};
    exported.state.last_processed_time = PsychTime{5};
    ImportOptions direct_import{};
    direct_import.destination_id = AgentId{WorldId{5}, 2};
    direct_import.import_time = PsychTime{10};
    direct_import.time_policy = ImportTimePolicy::PreserveAbsolute;
    direct_import.rng_policy = RngImportPolicy::PreserveState;
    auto imported_state = import_agent_state(exported, exported.origin_time_domain, direct_import);
    require(static_cast<bool>(imported_state), "direct import state");

    RuntimeConfig config{};
    config.runtime_id = RuntimeId{1};
    config.world_id = WorldId{1};
    config.time_domain = TimeDomain{TimeDomainId{1}, TimeScale{1, 1}};
    auto runtime_result = Runtime::create(config, registry);
    require(static_cast<bool>(runtime_result), "runtime create");
    auto runtime = std::move(*runtime_result.value_if());
    (void)runtime->config();
    (void)runtime->registry();
    (void)runtime->diagnostics();
    (void)runtime->agents();

    AgentInit init{};
    const AgentId first{WorldId{1}, 10};
    require(static_cast<bool>(runtime->create_agent(first, init, PsychTime{1})), "create agent");

    DerivedInit derived{};
    derived.source_agents.push_back(first);
    const AgentId second{WorldId{1}, 11};
    require(static_cast<bool>(runtime->create_agent_derived(second, derived, PsychTime{1})), "create derived agent");
    require(runtime->has_agent(first) && runtime->has_agent(second), "has agent");

    require(static_cast<bool>(runtime->begin_registry_staging()), "begin staging");
    require(runtime->has_registry_staging(), "has staging");
    require(static_cast<bool>(runtime->stage_domain_pack(pack)), "stage pack");
    require(static_cast<bool>(runtime->publish_registry()), "publish registry");
    runtime->discard_registry_staging();

    ObserveInput observation{};
    observation.agent = first;
    observation.proposition = proposition;
    observation.time.process_time = PsychTime{2};
    auto observed = runtime->observe(observation);
    require(static_cast<bool>(observed), "observe single");

    auto decision = runtime->decide(first, {}, PsychTime{3});
    require(static_cast<bool>(decision), "decide single");

    ReportInput report{};
    report.agent = first;
    report.action_type = ActionTypeId{1};
    report.success = true;
    report.process_time = PsychTime{4};
    require(static_cast<bool>(runtime->report(report)), "report single");
    require(static_cast<bool>(runtime->advance_to(first, PsychTime{5})), "advance single");

    auto observed_batch = runtime->observe_batch({ObserveBatchItem{0, ObserveInput{first, proposition, StimulusTime{PsychTime{6}, false, {}}}}});
    require(static_cast<bool>(observed_batch), "observe batch");
    auto decided_batch = runtime->decide_batch({DecideBatchItem{0, first, {}, PsychTime{7}, {}}});
    require(static_cast<bool>(decided_batch), "decide batch");
    report.process_time = PsychTime{8};
    auto reported_batch = runtime->report_batch({ReportBatchItem{0, report}});
    require(static_cast<bool>(reported_batch), "report batch");
    auto advanced_batch = runtime->advance_batch({AdvanceBatchItem{0, first, PsychTime{9}}});
    require(static_cast<bool>(advanced_batch), "advance batch");

    ReactInput react{};
    react.observation.agent = first;
    react.observation.proposition = proposition;
    react.observation.time.process_time = PsychTime{10};
    auto reacted = runtime->react(react);
    require(static_cast<bool>(reacted), "react");

    auto runtime_export = runtime->export_agent(first, PsychTime{11});
    require(static_cast<bool>(runtime_export), "runtime export");

    RuntimeConfig destination_config{};
    destination_config.runtime_id = RuntimeId{2};
    destination_config.world_id = WorldId{2};
    destination_config.time_domain = TimeDomain{TimeDomainId{2}, TimeScale{1, 1}};
    auto destination_result = Runtime::create(destination_config, registry);
    require(static_cast<bool>(destination_result), "destination runtime");
    auto destination = std::move(*destination_result.value_if());
    ImportOptions runtime_import{};
    runtime_import.destination_id = AgentId{WorldId{2}, 10};
    runtime_import.import_time = PsychTime{20};
    runtime_import.destination_seed_root = destination_config.seed_root;
    auto runtime_imported = destination->import_agent(*runtime_export.value_if(), runtime_import);
    require(static_cast<bool>(runtime_imported), "runtime import");

    auto unsupported_remove = runtime->remove_agent(second, PsychTime{11}, RemovePolicy::ValidateNoReferences);
    require(!unsupported_remove && unsupported_remove.error_if()->code == ErrorCode::UnsupportedOperation,
            "placeholder method remains callable");
    require(static_cast<bool>(runtime->remove_agent(second, PsychTime{11}, RemovePolicy::ForgetRuntimeOnly)), "remove derived agent");

    std::cout << "MSPA complete public API surface tests passed\n";
    return 0;
}
