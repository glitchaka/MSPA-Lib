#include "test.hpp"
#include "mspa/mspa.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace mspa;

namespace {

RegistrySnapshot system_registry() {
    RegistryBuilder builder;
    PredicateSchema p{};
    p.id = PredicateId(1);
    p.name_space = NamespaceId(1);
    p.default_relevance = 0.35f;
    p.default_consequence = 0.10f;
    p.optional_roles = {"a", "b"};
    require(builder.add_predicate(p), "register phase2 system predicate");
    return builder.publish();
}

RuntimeConfig config(std::uint64_t runtime_id, WorldId world = WorldId(1),
                     TimeDomain domain = {TimeDomainId(1), {60, 1}}) {
    RuntimeConfig cfg{};
    cfg.runtime_id = RuntimeId(runtime_id);
    cfg.world_id = world;
    cfg.time_domain = domain;
    cfg.seed_root = 0x1234abcd5678ef90ULL;
    cfg.diagnostics_enabled = true;
    return cfg;
}

std::unique_ptr<Runtime> make_runtime(const RuntimeConfig& cfg,
                                      const RegistrySnapshot& registry = RegistrySnapshot{}) {
    auto runtime = Runtime::create(cfg, registry);
    require(runtime, "create phase2 system runtime");
    return std::move(*runtime.value_if());
}

ObserveInput observe_input(AgentId agent, std::uint64_t process_time) {
    ObserveInput input{};
    input.agent = agent;
    input.proposition.predicate = PredicateId(1);
    input.proposition.name_space = NamespaceId(1);
    input.time.process_time = PsychTime(process_time);
    input.perceived_certainty = 0.80f;
    input.source_trust = 0.90f;
    return input;
}

std::vector<ActionOption> options(AgentId agent) {
    ActionOption a{};
    a.type = ActionTypeId(1);
    a.instance = ActionInstanceId(1);
    a.continuity = ActionContinuityKey(1);
    a.actor = agent;
    a.cost = 0.10f;
    a.risk = 0.10f;
    a.progress = 0.60f;
    ActionOption b = a;
    b.instance = ActionInstanceId(2);
    b.continuity = ActionContinuityKey(2);
    b.cost = 0.15f;
    b.progress = 0.45f;
    return {a, b};
}

std::vector<std::uint8_t> deterministic_scenario(bool add_other) {
    const auto registry = system_registry();
    auto runtime = make_runtime(config(400), registry);
    const AgentId target{WorldId(1), 10};
    if (add_other) {
        const AgentId other{WorldId(1), 999};
        require(runtime->create_agent(other, AgentInit{}, PsychTime(0)), "create unrelated agent");
        require(runtime->observe(observe_input(other, 5)), "observe unrelated agent");
        require(runtime->decide(other, options(other), PsychTime(6)), "decide unrelated agent");
    }
    require(runtime->create_agent(target, AgentInit{}, PsychTime(0)), "create deterministic target");
    require(runtime->observe(observe_input(target, 10)), "observe deterministic target");
    auto decision = runtime->decide(target, options(target), PsychTime(11));
    require(decision && decision.value_if()->has_selection, "decide deterministic target");
    ReportInput report{};
    report.agent = target;
    report.action_type = ActionTypeId(1);
    report.success = true;
    report.context_tags = {"phase2"};
    report.process_time = PsychTime(12);
    require(runtime->report(report), "report deterministic target");
    require(runtime->advance_to(target, PsychTime(100)), "advance deterministic target");
    auto bytes = runtime->export_agent_bytes(target, PsychTime(100));
    require(bytes, "export deterministic target");
    return *bytes.value_if();
}

BeliefState make_belief(std::uint64_t id) {
    BeliefState belief{};
    belief.id = BeliefId(id);
    belief.proposition.predicate = PredicateId(1);
    belief.proposition.name_space = NamespaceId(1);
    belief.certainty = 0.70f;
    belief.emotional_load = 0.10f;
    belief.updated_at = PsychTime(1);
    return belief;
}

MemoryState make_memory(std::uint64_t id) {
    MemoryState memory{};
    memory.id = MemoryId(id);
    memory.proposition.predicate = PredicateId(1);
    memory.proposition.name_space = NamespaceId(1);
    memory.importance = 0.70f;
    memory.emotional_load = -0.10f;
    memory.certainty = 0.80f;
    memory.event_time = PsychTime(1);
    memory.last_reactivated_at = PsychTime(1);
    return memory;
}

GoalState make_goal(std::uint64_t id) {
    GoalState goal{};
    goal.id = GoalId(id);
    goal.concept_id = ConceptId(id);
    goal.priority = 0.60f;
    goal.commitment = 0.70f;
    goal.created_at = PsychTime(1);
    goal.updated_at = PsychTime(1);
    return goal;
}

LearnedExpectation make_expectation(std::uint64_t action) {
    LearnedExpectation e{};
    e.action = ActionTypeId(action);
    e.context_tags = {"ctx"};
    e.outcome_tag = "success";
    e.attempts = 1;
    e.successes = 1;
    e.confidence = 0.75f;
    e.last_activation = PsychTime(1);
    return e;
}

void require_quota_failure(const Result<void>& result, const char* message) {
    require(!result, message);
    require(result.error_if() && result.error_if()->code == ErrorCode::QuotaExceeded,
            "quota failure must report QuotaExceeded");
}

ExportedAgent complex_exported_agent() {
    ExportedAgent exported{};
    exported.origin_time_domain = {TimeDomainId(10), {10, 1}};
    exported.exported_at = PsychTime(100);
    auto& state = exported.state;
    state.id = {WorldId(1), 500};
    state.personality = {0.2f, 0.8f, 0.7f, 0.9f, 0.4f, 0.6f};
    state.values.push_back({ConceptId(1), 0.75f});

    BeliefState belief = make_belief(1);
    belief.proposition.arguments.push_back({"subject", "alpha", false, {}, true, ConceptId(2)});
    belief.proposition.qualifiers["scope"] = "local";
    belief.proposition.has_time = true;
    belief.proposition.time = PsychTime(60);
    belief.proposition.provenance = "phase2";
    belief.updated_at = PsychTime(70);
    state.beliefs.push_back(belief);

    state.emotions = {
        {BasicEmotion::Joy, 0.11f, PsychTime(50), PsychTime(75)},
        {BasicEmotion::Sadness, 0.12f, PsychTime(51), PsychTime(75)},
        {BasicEmotion::Fear, 0.40f, PsychTime(52), PsychTime(75)},
        {BasicEmotion::Anger, 0.13f, PsychTime(53), PsychTime(75)},
        {BasicEmotion::Anxiety, 0.20f, PsychTime(54), PsychTime(75)},
        {BasicEmotion::Aversion, 0.14f, PsychTime(55), PsychTime(75)}
    };
    state.mood = {-0.20f, 0.40f, PsychTime(80)};
    state.goals.push_back(make_goal(1));
    state.goals.back().created_at = PsychTime(40);
    state.goals.back().updated_at = PsychTime(85);

    MemoryState memory = make_memory(1);
    memory.proposition.has_time = true;
    memory.proposition.time = PsychTime(55);
    memory.participants.push_back({WorldId(1), 501});
    memory.reactivations = 2;
    memory.event_time = PsychTime(30);
    memory.last_reactivated_at = PsychTime(65);
    state.memories.push_back(memory);

    state.relationships.push_back({{WorldId(1), 502}, 0.30f, 0.40f, 0.50f, 0.60f});
    auto learned = make_expectation(7);
    learned.context_tags = {"rain", "night"};
    learned.attempts = 3;
    learned.successes = 2;
    learned.confidence = 0.70f;
    learned.last_activation = PsychTime(45);
    state.learned_expectations.push_back(learned);
    state.current_intent = ActionContinuityKey(9);
    state.rng_state.state = 0xabcdef1234567890ULL;
    state.last_processed_time = PsychTime(90);
    return exported;
}

} // namespace

int main() {
    // A1: deterministic bytes and agent independence.
    const auto deterministic_a = deterministic_scenario(false);
    const auto deterministic_b = deterministic_scenario(false);
    const auto deterministic_with_other = deterministic_scenario(true);
    require(deterministic_a == deterministic_b, "A1 same scenario must export identical bytes");
    require(deterministic_a == deterministic_with_other,
            "A1 unrelated agent must not perturb deterministic target state");

    // A1/A15 canonical batch ordering with more than one agent.
    const auto registry = system_registry();
    auto batch_runtime = make_runtime(config(401), registry);
    const AgentId low{WorldId(1), 1};
    const AgentId high{WorldId(1), 2};
    require(batch_runtime->create_agent(low, AgentInit{}, PsychTime(0)), "create low batch agent");
    require(batch_runtime->create_agent(high, AgentInit{}, PsychTime(0)), "create high batch agent");
    ObserveBatchItem high_first{};
    high_first.input_sequence = 1;
    high_first.input = observe_input(high, 10);
    ObserveBatchItem low_second{};
    low_second.input_sequence = 1;
    low_second.input = observe_input(low, 10);
    auto batch_outputs = batch_runtime->observe_batch({high_first, low_second});
    require(batch_outputs && batch_outputs.value_if()->size() == 2, "multiagent batch succeeds");
    require((*batch_outputs.value_if())[0].agent == low && (*batch_outputs.value_if())[1].agent == high,
            "A1 batch output must use canonical AgentId ordering");

    auto individual_runtime = make_runtime(config(401), registry);
    require(individual_runtime->create_agent(low, AgentInit{}, PsychTime(0)), "create low individual agent");
    require(individual_runtime->create_agent(high, AgentInit{}, PsychTime(0)), "create high individual agent");
    require(individual_runtime->observe(observe_input(low, 10)), "individual low observe");
    require(individual_runtime->observe(observe_input(high, 10)), "individual high observe");
    auto batch_low = batch_runtime->export_agent_bytes(low, PsychTime(10));
    auto batch_high = batch_runtime->export_agent_bytes(high, PsychTime(10));
    auto individual_low = individual_runtime->export_agent_bytes(low, PsychTime(10));
    auto individual_high = individual_runtime->export_agent_bytes(high, PsychTime(10));
    require(batch_low && batch_high && individual_low && individual_high, "export batch/individual states");
    require(*batch_low.value_if() == *individual_low.value_if() &&
                *batch_high.value_if() == *individual_high.value_if(),
            "A1 multiagent batch state must equal individual calls");

    // A9: process time regression rejected, historical event time accepted.
    auto time_runtime = make_runtime(config(402), registry);
    const AgentId time_agent{WorldId(1), 20};
    require(time_runtime->create_agent(time_agent, AgentInit{}, PsychTime(10)), "create time audit agent");
    auto late = observe_input(time_agent, 20);
    late.time.has_event_time = true;
    late.time.event_time = PsychTime(5);
    require(time_runtime->observe(late), "A9 historical event_time must be accepted");
    auto backwards = late;
    backwards.time.process_time = PsychTime(19);
    auto regression = time_runtime->observe(backwards);
    require(!regression && regression.error_if() && regression.error_if()->code == ErrorCode::TimeRegression,
            "A9 backward process_time must return TimeRegression");

    // A9: RebaseRelative preserves every populated psychological timestamp.
    auto temporal = complex_exported_agent();
    ImportOptions import_options{};
    import_options.destination_id = {WorldId(2), 500};
    import_options.import_time = PsychTime(1000);
    import_options.time_policy = ImportTimePolicy::RebaseRelative;
    import_options.rng_policy = RngImportPolicy::RekeyForDestination;
    import_options.destination_seed_root = 777;
    const TimeDomain destination_domain{TimeDomainId(20), {20, 1}};
    auto imported = import_agent_state(temporal, destination_domain, import_options);
    require(imported, "A9 rebase complex public state");
    const auto& rebased = imported.value_if()->state;
    require(rebased.last_processed_time == PsychTime(980), "A9 rebase last_processed_time");
    require(rebased.mood.updated_at == PsychTime(960), "A9 rebase mood timestamp");
    require(rebased.beliefs.front().updated_at == PsychTime(940), "A9 rebase belief timestamp");
    require(rebased.beliefs.front().proposition.time == PsychTime(920), "A9 rebase belief proposition time");
    require(rebased.emotions.front().started_at == PsychTime(900), "A9 rebase emotion start");
    require(rebased.emotions.front().updated_at == PsychTime(950), "A9 rebase emotion update");
    require(rebased.goals.front().created_at == PsychTime(880), "A9 rebase goal create");
    require(rebased.goals.front().updated_at == PsychTime(970), "A9 rebase goal update");
    require(rebased.memories.front().event_time == PsychTime(860), "A9 rebase memory event");
    require(rebased.memories.front().last_reactivated_at == PsychTime(930), "A9 rebase memory reactivation");
    require(rebased.memories.front().proposition.time == PsychTime(910), "A9 rebase memory proposition time");
    require(rebased.learned_expectations.front().last_activation == PsychTime(890),
            "A9 rebase learning activation");

    ImportOptions second_import = import_options;
    second_import.destination_id = {WorldId(2), 501};
    auto imported_second = import_agent_state(temporal, destination_domain, second_import);
    require(imported_second, "A9 second rekey import");
    require(rebased.rng_state.state != temporal.state.rng_state.state,
            "A9 RekeyForDestination must not preserve source RNG");
    require(rebased.rng_state.state != imported_second.value_if()->state.rng_state.state,
            "A9 two destination AgentIds must not share RNG state");

    // A10: exact binary round-trip and migration from previous schema marker 0.
    const auto complex = complex_exported_agent();
    auto encoded = serialize_agent(complex);
    require(encoded, "A10 serialize complex agent");
    auto decoded = deserialize_agent(*encoded.value_if());
    require(decoded, "A10 deserialize complex agent");
    auto reencoded = serialize_agent(*decoded.value_if());
    require(reencoded, "A10 reserialize complex agent");
    require(*encoded.value_if() == *reencoded.value_if(), "A10 complex round-trip must be byte exact");

    auto legacy = complex;
    legacy.state.schema_version = 0;
    auto legacy_bytes = serialize_agent(legacy);
    require(legacy_bytes, "A10 serialize supported legacy schema");
    auto migrated = deserialize_agent(*legacy_bytes.value_if());
    require(migrated, "A10 migrate supported previous schema");
    require(migrated.value_if()->state.schema_version == state_schema_version,
            "A10 migration must reach current state_schema_version");

    // A11: runtime isolation, duplicate IDs, reference-safe removal and world isolation.
    auto isolated_a = make_runtime(config(403));
    auto isolated_b = make_runtime(config(404));
    const AgentId isolated_id{WorldId(1), 30};
    require(isolated_a->create_agent(isolated_id, AgentInit{}, PsychTime(0)), "create isolated A agent");
    require(isolated_b->create_agent(isolated_id, AgentInit{}, PsychTime(0)), "create isolated B agent");
    auto duplicate = isolated_a->create_agent(isolated_id, AgentInit{}, PsychTime(0));
    require(!duplicate && duplicate.error_if() && duplicate.error_if()->code == ErrorCode::DuplicateAgent,
            "A11 duplicate agent must return DuplicateAgent");
    require(isolated_a->remove_agent(isolated_id, PsychTime(1)), "remove isolated A agent");
    require(!isolated_a->has_agent(isolated_id) && isolated_b->has_agent(isolated_id),
            "A11 runtimes must not interfere");

    auto references = make_runtime(config(405));
    const AgentId referenced{WorldId(1), 40};
    const AgentId holder{WorldId(1), 41};
    require(references->create_agent(referenced, AgentInit{}, PsychTime(0)), "create referenced agent");
    AgentInit holder_init{};
    holder_init.relationships.push_back({referenced, 0.1f, 0.2f, 0.3f, 0.4f});
    require(references->create_agent(holder, holder_init, PsychTime(0)), "create holder agent");
    auto protected_remove = references->remove_agent(referenced, PsychTime(1), RemovePolicy::ValidateNoReferences);
    require(!protected_remove && protected_remove.error_if() &&
                protected_remove.error_if()->code == ErrorCode::InvalidState,
            "A11 ValidateNoReferences must reject referenced removal");
    require(references->has_agent(referenced), "A11 failed protected removal must preserve agent");

    auto world_one = make_runtime(config(406, WorldId(1)));
    auto wrong_world = world_one->create_agent({WorldId(2), 1}, AgentInit{}, PsychTime(0));
    require(!wrong_world && wrong_world.error_if() && wrong_world.error_if()->code == ErrorCode::InvalidArgument,
            "A11 runtime must reject AgentId from another world");
    auto world_two = make_runtime(config(407, WorldId(2)));
    require(world_one->create_agent({WorldId(1), 1}, AgentInit{}, PsychTime(0)), "create world-one local id");
    require(world_two->create_agent({WorldId(2), 1}, AgentInit{}, PsychTime(0)), "create world-two same local id");

    // A13: max_agents.
    {
        auto cfg = config(410);
        cfg.limits.max_agents = 1;
        auto runtime = make_runtime(cfg);
        const AgentId first{WorldId(1), 1};
        const AgentId second{WorldId(1), 2};
        require(runtime->create_agent(first, AgentInit{}, PsychTime(0)), "quota first agent");
        require_quota_failure(runtime->create_agent(second, AgentInit{}, PsychTime(0)), "max_agents overflow");
        require(runtime->has_agent(first) && !runtime->has_agent(second),
                "max_agents failure must not corrupt existing state");
    }

    // A13: vector/state quotas at admission.
    {
        auto cfg = config(411);
        cfg.limits.max_values_per_agent = 1;
        auto runtime = make_runtime(cfg);
        AgentInit init{};
        init.values = {{ConceptId(1), 0.5f}, {ConceptId(2), 0.5f}};
        require_quota_failure(runtime->create_agent({WorldId(1), 1}, init, PsychTime(0)), "values quota");
        require(!runtime->has_agent({WorldId(1), 1}), "values quota failure must not insert agent");
    }
    {
        auto cfg = config(412);
        cfg.limits.max_beliefs_per_agent = 1;
        auto runtime = make_runtime(cfg);
        AgentInit init{};
        init.beliefs = {make_belief(1), make_belief(2)};
        require_quota_failure(runtime->create_agent({WorldId(1), 1}, init, PsychTime(0)), "belief quota");
    }
    {
        auto cfg = config(413);
        cfg.limits.max_emotions_per_agent = 6;
        auto runtime = make_runtime(cfg);
        AgentInit init{};
        init.emotions = {
            {BasicEmotion::Joy, 0.1f, {}, {}}, {BasicEmotion::Sadness, 0.1f, {}, {}},
            {BasicEmotion::Fear, 0.1f, {}, {}}, {BasicEmotion::Anger, 0.1f, {}, {}},
            {BasicEmotion::Anxiety, 0.1f, {}, {}}, {BasicEmotion::Aversion, 0.1f, {}, {}},
            {BasicEmotion::Joy, 0.1f, {}, {}}
        };
        require_quota_failure(runtime->create_agent({WorldId(1), 1}, init, PsychTime(0)), "emotion quota");
    }
    {
        auto cfg = config(414);
        cfg.limits.max_memories_per_agent = 1;
        auto runtime = make_runtime(cfg);
        AgentInit init{};
        init.memories = {make_memory(1), make_memory(2)};
        require_quota_failure(runtime->create_agent({WorldId(1), 1}, init, PsychTime(0)), "memory quota");
    }
    {
        auto cfg = config(415);
        cfg.limits.max_goals_per_agent = 1;
        auto runtime = make_runtime(cfg);
        AgentInit init{};
        init.goals = {make_goal(1), make_goal(2)};
        require_quota_failure(runtime->create_agent({WorldId(1), 1}, init, PsychTime(0)), "goal quota");
    }
    {
        auto cfg = config(416);
        cfg.limits.max_relationships_per_agent = 1;
        auto runtime = make_runtime(cfg);
        AgentInit init{};
        init.relationships = {{{WorldId(1), 2}, 0, 0, 0, 0}, {{WorldId(1), 3}, 0, 0, 0, 0}};
        require_quota_failure(runtime->create_agent({WorldId(1), 1}, init, PsychTime(0)), "relationship quota");
    }
    {
        auto cfg = config(417);
        cfg.limits.max_learning_patterns_per_agent = 1;
        auto runtime = make_runtime(cfg);
        AgentInit init{};
        init.learned_expectations = {make_expectation(1), make_expectation(2)};
        require_quota_failure(runtime->create_agent({WorldId(1), 1}, init, PsychTime(0)), "learning quota");
    }

    // A13: proposition and string limits must apply to OBSERVE before mutation.
    {
        auto cfg = config(418);
        cfg.limits.max_proposition_args = 1;
        auto runtime = make_runtime(cfg, registry);
        const AgentId agent{WorldId(1), 1};
        require(runtime->create_agent(agent, AgentInit{}, PsychTime(0)), "create proposition quota agent");
        auto before = runtime->export_agent_bytes(agent, PsychTime(0));
        require(before, "export before proposition quota failure");
        auto input = observe_input(agent, 1);
        input.proposition.arguments = {{"a", "x", false, {}, false, {}},
                                       {"b", "y", false, {}, false, {}}};
        auto overflow = runtime->observe(input);
        require(!overflow && overflow.error_if() && overflow.error_if()->code == ErrorCode::QuotaExceeded,
                "max_proposition_args overflow must return QuotaExceeded");
        auto after = runtime->export_agent_bytes(agent, PsychTime(0));
        require(after && *before.value_if() == *after.value_if(),
                "proposition quota failure must not mutate agent state");
    }
    {
        auto cfg = config(419);
        cfg.limits.max_string_bytes = 4;
        auto runtime = make_runtime(cfg, registry);
        const AgentId agent{WorldId(1), 1};
        require(runtime->create_agent(agent, AgentInit{}, PsychTime(0)), "create string quota agent");
        auto before = runtime->export_agent_bytes(agent, PsychTime(0));
        require(before, "export before string quota failure");
        auto input = observe_input(agent, 1);
        input.proposition.arguments = {{"a", "12345", false, {}, false, {}}};
        auto overflow = runtime->observe(input);
        require(!overflow && overflow.error_if() && overflow.error_if()->code == ErrorCode::QuotaExceeded,
                "max_string_bytes overflow must return QuotaExceeded");
        auto after = runtime->export_agent_bytes(agent, PsychTime(0));
        require(after && *before.value_if() == *after.value_if(),
                "string quota failure must not mutate agent state");
    }

    // A13: serialized byte limit.
    {
        auto cfg = config(420);
        cfg.limits.max_serialized_bytes = 64;
        auto runtime = make_runtime(cfg);
        const AgentId agent{WorldId(1), 1};
        require(runtime->create_agent(agent, AgentInit{}, PsychTime(0)), "create serialization quota agent");
        auto bytes = runtime->export_agent_bytes(agent, PsychTime(0));
        require(!bytes && bytes.error_if() && bytes.error_if()->code == ErrorCode::QuotaExceeded,
                "max_serialized_bytes overflow must return QuotaExceeded");
        require(runtime->has_agent(agent), "serialization quota failure must not remove/corrupt agent");
    }

    // A13: diagnostics is a bounded retention quota; reaching it cannot change simulation result (A7).
    {
        auto cfg = config(421);
        cfg.limits.max_diagnostics = 2;
        auto runtime = make_runtime(cfg);
        const AgentId agent{WorldId(1), 1};
        require(runtime->create_agent(agent, AgentInit{}, PsychTime(0)), "create diagnostics quota agent");
        ActionOption option{};
        option.type = ActionTypeId(1);
        option.instance = ActionInstanceId(1);
        option.actor = agent;
        for (std::uint64_t t = 1; t <= 3; ++t)
            require(runtime->decide(agent, {option}, PsychTime(t)), "produce bounded diagnostic trace");
        require(runtime->diagnostics().traces().size() == 2,
                "max_diagnostics must cap retained traces without growing beyond limit");
    }

    // A13: invalid emotion capacity and Domain Packs cannot elevate runtime limits.
    {
        auto cfg = config(422);
        cfg.limits.max_emotions_per_agent = 5;
        auto invalid = Runtime::create(cfg);
        require(!invalid && invalid.error_if() && invalid.error_if()->code == ErrorCode::InvalidArgument,
                "max_emotions_per_agent below six must be rejected");
    }
    {
        auto cfg = config(423);
        cfg.limits.max_beliefs_per_agent = 1;
        auto runtime = make_runtime(cfg);
        DomainPack pack{};
        pack.metadata.id = PackId(1);
        pack.metadata.name_space = NamespaceId(2);
        pack.metadata.name = "phase2-pack";
        PredicateSchema p{};
        p.id = PredicateId(99);
        p.name_space = NamespaceId(2);
        pack.predicates.push_back(p);
        require(runtime->stage_domain_pack(pack), "stage valid phase2 pack");
        require(runtime->publish_registry(), "publish valid phase2 pack");
        require(runtime->config().limits.max_beliefs_per_agent == 1,
                "Domain Pack must not elevate RuntimeLimits");
    }

    std::cout << "AUDIT_PHASE2_SYSTEM PASS"
              << " deterministic_bytes=" << deterministic_a.size()
              << " canonical_first=" << (*batch_outputs.value_if())[0].agent.local_id
              << " roundtrip_bytes=" << encoded.value_if()->size()
              << " rekey=" << rebased.rng_state.state << "/" << imported_second.value_if()->state.rng_state.state
              << " diagnostics_cap=2"
              << "\n";
}
