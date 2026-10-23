#include <cstdlib>
#include <iostream>
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
    PersonalitySystem personality_system;
    Personality personality{};
    require(static_cast<bool>(personality_system.normalize(personality)), "personality normalize");
    (void)personality_system.derive(personality);

    AgentPsychState state{};
    state.id = AgentId{WorldId{1}, 1};
    ValueSystem values;
    require(static_cast<bool>(values.set(state, ConceptId{1}, 0.7f)), "value set");
    (void)values.intensity(state, ConceptId{1});
    require(static_cast<bool>(values.erase(state, ConceptId{1})), "value erase");

    MoodSystem mood;
    mood.update(state, 0.1f, 0.1f, PsychTime{1});
    mood.decay(state, PsychTime{2});
    mood.reset(state, PsychTime{3});

    MemorySystem memory;
    auto memory_id = memory.remember(state, PredicateId{1}, 0.5f, PsychTime{1});
    require(static_cast<bool>(memory_id), "memory remember");
    ReappraisalSystem reappraisal;
    require(static_cast<bool>(reappraisal.reappraise_memory(state, *memory_id.value_if(), 0.2f, 0.9f)), "reappraise");
    require(static_cast<bool>(reappraisal.reactivate_memory(state, *memory_id.value_if())), "reactivate");

    InferenceEngine inference;
    InferenceRule rule{};
    rule.id = 1;
    rule.conclusion = PredicateId{2};
    rule.conclusion_namespace = NamespaceId{1};
    require(static_cast<bool>(inference.add_rule(rule)), "inference add rule");
    require(inference.rule_count() == 1, "inference rule count");
    auto hypotheses = inference.infer({}, RegistrySnapshot{});
    require(static_cast<bool>(hypotheses), "inference run");
    inference.clear_rules();

    ResponseCurve curve;
    require(static_cast<bool>(curve.set_points({{0.0f, 0.0f}, {1.0f, 1.0f}})), "curve set");
    (void)curve.evaluate(0.5f);
    curve.clear();

    ConsiderationRegistry considerations;
    ConsiderationDefinition definition{};
    definition.id = "test";
    require(static_cast<bool>(considerations.add(std::move(definition))), "consideration add");
    require(considerations.find("test") != nullptr, "consideration find");
    require(static_cast<bool>(considerations.remove("test")), "consideration remove");
    considerations.clear();

    DomainPack pack{};
    pack.metadata.id = PackId{1};
    pack.metadata.name_space = NamespaceId{1};
    pack.metadata.name = "neutral";
    require(static_cast<bool>(validate_domain_pack(pack)), "pack validate");
    RegistryBuilder builder;
    require(static_cast<bool>(apply_domain_pack(builder, pack)), "pack apply");

    RuntimeConfig config{};
    config.runtime_id = RuntimeId{1};
    config.world_id = WorldId{1};
    config.time_domain = TimeDomain{TimeDomainId{1}, TimeScale{1, 1}};
    auto runtime_result = Runtime::create(config, builder.publish());
    require(static_cast<bool>(runtime_result), "runtime create");
    auto runtime = std::move(*runtime_result.value_if());

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
    observation.proposition.predicate = PredicateId{1};
    observation.proposition.name_space = NamespaceId{1};
    observation.time.process_time = PsychTime{2};
    auto observed_batch = runtime->observe_batch({ObserveBatchItem{0, observation}});
    require(static_cast<bool>(observed_batch), "observe batch");

    auto decided_batch = runtime->decide_batch({DecideBatchItem{0, first, {}, PsychTime{3}, {}}});
    require(static_cast<bool>(decided_batch), "decide batch");

    ReportInput report{};
    report.agent = first;
    report.action_type = ActionTypeId{1};
    report.success = true;
    report.process_time = PsychTime{4};
    auto reported_batch = runtime->report_batch({ReportBatchItem{0, report}});
    require(static_cast<bool>(reported_batch), "report batch");

    auto advanced_batch = runtime->advance_batch({AdvanceBatchItem{0, first, PsychTime{5}}});
    require(static_cast<bool>(advanced_batch), "advance batch");

    ReactInput react{};
    react.observation.agent = first;
    react.observation.proposition.predicate = PredicateId{1};
    react.observation.proposition.name_space = NamespaceId{1};
    react.observation.time.process_time = PsychTime{6};
    auto reacted = runtime->react(react);
    require(static_cast<bool>(reacted), "react");

    auto unsupported_remove = runtime->remove_agent(second, PsychTime{7}, RemovePolicy::ValidateNoReferences);
    require(!unsupported_remove && unsupported_remove.error_if()->code == ErrorCode::UnsupportedOperation,
            "explicit placeholder method must still be callable");
    require(static_cast<bool>(runtime->remove_agent(second, PsychTime{7}, RemovePolicy::ForgetRuntimeOnly)), "remove derived agent");

    std::cout << "MSPA API surface tests passed\n";
    return 0;
}
