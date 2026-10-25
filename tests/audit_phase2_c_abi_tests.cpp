#include "test.hpp"
#include "mspa/c_api.h"
#include "mspa/mspa.hpp"
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

using namespace mspa;

namespace {

mspa_runtime* make_c_runtime(std::uint64_t runtime_id, std::uint64_t world,
                             std::uint64_t domain_id, std::uint64_t ticks_num,
                             std::uint64_t ticks_den, std::uint64_t seed,
                             bool diagnostics = false) {
    mspa_runtime_config cfg{};
    cfg.runtime_id = runtime_id;
    cfg.world_id = world;
    cfg.time_domain_id = domain_id;
    cfg.ticks_per_second_num = ticks_num;
    cfg.ticks_per_second_den = ticks_den;
    cfg.seed_root = seed;
    cfg.diagnostics_enabled = diagnostics ? 1 : 0;
    mspa_runtime* runtime = nullptr;
    require(mspa_runtime_create(&cfg, &runtime) == MSPA_OK && runtime != nullptr, "create C ABI runtime");
    return runtime;
}

std::unique_ptr<Runtime> make_cpp_runtime(std::uint64_t runtime_id, std::uint64_t world,
                                          std::uint64_t domain_id, std::uint64_t ticks_num,
                                          std::uint64_t ticks_den, std::uint64_t seed,
                                          bool diagnostics = false) {
    RuntimeConfig cfg{};
    cfg.runtime_id = RuntimeId(runtime_id);
    cfg.world_id = WorldId(world);
    cfg.time_domain = {TimeDomainId(domain_id), {ticks_num, ticks_den}};
    cfg.seed_root = seed;
    cfg.diagnostics_enabled = diagnostics;
    auto runtime = Runtime::create(cfg);
    require(runtime, "create C++ equivalence runtime");
    return std::move(*runtime.value_if());
}

std::vector<std::uint8_t> export_c(mspa_runtime* runtime, mspa_agent_id agent, std::uint64_t at) {
    mspa_buffer buffer{};
    require(mspa_agent_export(runtime, agent, at, &buffer) == MSPA_OK, "C ABI export agent");
    std::vector<std::uint8_t> bytes;
    if (buffer.size != 0) bytes.assign(buffer.data, buffer.data + buffer.size);
    mspa_buffer_free(&buffer);
    return bytes;
}

struct CScenarioResult {
    std::vector<std::uint8_t> bytes{};
    std::uint64_t selected{0};
    float confidence{0.0f};
};

CScenarioResult run_c_learning_scenario() {
    mspa_runtime* runtime = make_c_runtime(500, 7, 1, 60, 1, 555, false);
    const mspa_agent_id agent{7, 1};
    require(mspa_agent_create(runtime, agent, 0) == MSPA_OK, "C create learning agent");
    const char* rain_tags[] = {"rain"};
    for (std::uint64_t i = 1; i <= 8; ++i) {
        require(mspa_report_ex(runtime, agent, 10, 1, rain_tags, 1, nullptr, 0, i) == MSPA_OK,
                "C report repeated success");
        require(mspa_report_ex(runtime, agent, 11, 0, rain_tags, 1, nullptr, 0, i) == MSPA_OK,
                "C report repeated failure");
    }

    mspa_action_option options[2]{};
    options[0].type_id = 11;
    options[0].instance_id = 11;
    options[0].context_stability = 1.0f;
    options[0].context_tags = rain_tags;
    options[0].context_tag_count = 1;
    options[1].type_id = 10;
    options[1].instance_id = 10;
    options[1].context_stability = 1.0f;
    options[1].context_tags = rain_tags;
    options[1].context_tag_count = 1;
    mspa_decision_result decision{};
    require(mspa_decide(runtime, agent, options, 2, 9, 0, &decision) == MSPA_OK && decision.has_selection,
            "C learning DECIDE");

    CScenarioResult result{};
    result.selected = decision.selected_action;
    result.confidence = decision.confidence;
    result.bytes = export_c(runtime, agent, 9);
    mspa_runtime_destroy(runtime);
    return result;
}

struct CppScenarioResult {
    std::vector<std::uint8_t> bytes{};
    ActionInstanceId selected{};
    float confidence{0.0f};
};

CppScenarioResult run_cpp_learning_scenario() {
    auto runtime = make_cpp_runtime(500, 7, 1, 60, 1, 555, false);
    const AgentId agent{WorldId(7), 1};
    require(runtime->create_agent(agent, AgentInit{}, PsychTime(0)), "C++ create learning agent");
    for (std::uint64_t i = 1; i <= 8; ++i) {
        ReportInput good{};
        good.agent = agent;
        good.action_type = ActionTypeId(10);
        good.success = true;
        good.context_tags = {"rain"};
        good.process_time = PsychTime(i);
        require(runtime->report(good), "C++ report repeated success");
        ReportInput bad = good;
        bad.action_type = ActionTypeId(11);
        bad.success = false;
        require(runtime->report(bad), "C++ report repeated failure");
    }

    ActionOption bad{};
    bad.type = ActionTypeId(11);
    bad.instance = ActionInstanceId(11);
    bad.actor = agent;
    bad.context_tags = {"rain"};
    ActionOption good = bad;
    good.type = ActionTypeId(10);
    good.instance = ActionInstanceId(10);
    auto decision = runtime->decide(agent, {bad, good}, PsychTime(9));
    require(decision && decision.value_if()->has_selection, "C++ learning DECIDE");
    auto bytes = runtime->export_agent_bytes(agent, PsychTime(9));
    require(bytes, "C++ learning export");

    CppScenarioResult result{};
    result.selected = decision.value_if()->selected_action;
    result.confidence = decision.value_if()->confidence;
    result.bytes = *bytes.value_if();
    return result;
}

} // namespace

int main() {
    // A12/A1/A4: identical public scenario through C and C++.
    const auto cpp = run_cpp_learning_scenario();
    const auto c_first = run_c_learning_scenario();
    const auto c_second = run_c_learning_scenario();
    require(cpp.selected.value() == c_first.selected && c_first.selected == 10,
            "A12 C and C++ must choose the same learned action");
    require(c_first.bytes == c_second.bytes, "A12 C ABI repeated scenario must be deterministic");
    require(cpp.bytes == c_first.bytes, "A12 C ABI state bytes must match equivalent C++ scenario");
    require(cpp.confidence == c_first.confidence, "A12 C and C++ decision confidence must match");

    // A12/A9: default C import is RebaseRelative + RekeyForDestination and must match C++.
    auto source = make_cpp_runtime(501, 7, 10, 10, 1, 777, false);
    const AgentId source_agent{WorldId(7), 5};
    require(source->create_agent(source_agent, AgentInit{}, PsychTime(0)), "create cross-scale source agent");
    require(source->advance_to(source_agent, PsychTime(90)), "advance cross-scale source agent");
    auto source_bytes = source->export_agent_bytes(source_agent, PsychTime(100));
    require(source_bytes, "export cross-scale source bytes");

    auto cpp_destination = make_cpp_runtime(502, 8, 20, 20, 1, 999, false);
    ImportOptions import_options{};
    import_options.destination_id = {WorldId(8), 6};
    import_options.import_time = PsychTime(1000);
    import_options.time_policy = ImportTimePolicy::RebaseRelative;
    import_options.rng_policy = RngImportPolicy::RekeyForDestination;
    import_options.destination_seed_root = 999;
    auto cpp_import = cpp_destination->import_agent_bytes(*source_bytes.value_if(), import_options);
    require(cpp_import, "C++ cross-scale import");
    auto cpp_imported_bytes = cpp_destination->export_agent_bytes({WorldId(8), 6}, PsychTime(1000));
    require(cpp_imported_bytes, "C++ cross-scale export");

    mspa_runtime* c_destination = make_c_runtime(502, 8, 20, 20, 1, 999, false);
    const mspa_agent_id c_destination_agent{8, 6};
    require(mspa_agent_import(c_destination, source_bytes.value_if()->data(), source_bytes.value_if()->size(),
                              c_destination_agent, 1000) == MSPA_OK,
            "C cross-scale import");
    const auto c_imported_bytes = export_c(c_destination, c_destination_agent, 1000);
    require(*cpp_imported_bytes.value_if() == c_imported_bytes,
            "A12 C RebaseRelative/Rekey import must match C++ bytes");
    auto decoded_c_import = deserialize_agent(c_imported_bytes);
    require(decoded_c_import, "decode C imported state through public serializer API");
    require(decoded_c_import.value_if()->state.last_processed_time == PsychTime(980),
            "A12 C import preserves cross-scale relative age");
    require(decoded_c_import.value_if()->state.rng_state.state == cpp_import.value_if()->state.rng_state.state,
            "A12 C import rekeys RNG exactly as C++");
    mspa_runtime_destroy(c_destination);

    // A12/A9: historical event time accepted and process time regression maps to C status.
    mspa_runtime* c_time = make_c_runtime(503, 9, 1, 60, 1, 123, false);
    const mspa_agent_id c_time_agent{9, 1};
    require(mspa_agent_create(c_time, c_time_agent, 10) == MSPA_OK, "C create time agent");
    mspa_observe_input observation{};
    observation.agent = c_time_agent;
    observation.predicate_id = 1;
    observation.namespace_id = 1;
    observation.positive = 1;
    observation.process_time = 20;
    observation.has_event_time = 1;
    observation.event_time = 5;
    observation.perceived_certainty = 0.8f;
    observation.source_trust = 0.9f;
    mspa_observe_result observed{};
    require(mspa_observe(c_time, &observation, &observed) == MSPA_OK,
            "A12 C historical event_time must be accepted");
    observation.process_time = 19;
    require(mspa_observe(c_time, &observation, &observed) == MSPA_TIME_REGRESSION,
            "A12 C backward process_time must map to MSPA_TIME_REGRESSION");

    // A12: error paths return status and do not leak C++ exceptions through extern C.
    bool exception_escaped = false;
    try {
        require(mspa_agent_create(nullptr, c_time_agent, 0) == MSPA_INVALID_ARGUMENT,
                "C null runtime returns invalid argument");
        require(mspa_agent_create(c_time, c_time_agent, 20) == MSPA_DUPLICATE_AGENT,
                "C duplicate agent returns duplicate status");
        const std::uint8_t malformed[] = {0, 1, 2, 3};
        require(mspa_agent_import(c_time, malformed, sizeof(malformed), {9, 2}, 30) != MSPA_OK,
                "C malformed import returns an error status");
    } catch (...) {
        exception_escaped = true;
    }
    require(!exception_escaped, "A12 no C ABI error path may propagate an exception");
    mspa_runtime_destroy(c_time);

    std::cout << "AUDIT_PHASE2_C_ABI PASS"
              << " learned_action=" << c_first.selected
              << " bytes=" << c_first.bytes.size()
              << " imported_bytes=" << c_imported_bytes.size()
              << "\n";
}
