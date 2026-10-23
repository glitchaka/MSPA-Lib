#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "mspa/base/result.hpp"
#include "mspa/decision/decision.hpp"
#include "mspa/diagnostics/trace.hpp"
#include "mspa/model/agent_state.hpp"
#include "mspa/persistence/state_transfer.hpp"
#include "mspa/psychology/engine.hpp"
#include "mspa/runtime/agent_store.hpp"
#include "mspa/semantics/domain_pack.hpp"
#include "mspa/semantics/registry.hpp"
#include "mspa/validation/limits.hpp"

namespace mspa {

enum class Urgency : std::uint8_t { None, Normal, Soon, Immediate };
enum class RemovePolicy : std::uint8_t { ForgetRuntimeOnly, ValidateNoReferences };
enum class TraitBlendPolicy : std::uint8_t { Mean, FirstSource };
enum class InheritedValuePolicy : std::uint8_t { None, Mean };

struct RuntimeConfig {
    RuntimeId runtime_id{};
    WorldId world_id{};
    TimeDomain time_domain{};
    std::uint64_t seed_root{0};
    RuntimeLimits limits{};
    bool diagnostics_enabled{true};
};

struct AgentInit {
    Personality personality{};
    std::vector<ValueEntry> values{};
    std::vector<BeliefState> beliefs{};
    std::vector<GoalState> goals{};
    std::vector<MemoryState> memories{};
    std::vector<RelationshipState> relationships{};
    bool use_custom_rng_state{false};
    RngState rng_state{};
};

struct DerivedInit {
    std::vector<AgentId> source_agents{};
    TraitBlendPolicy trait_blend_policy{TraitBlendPolicy::Mean};
    InheritedValuePolicy inherited_value_policy{InheritedValuePolicy::Mean};
    std::uint64_t variation_seed{0};
    float variation_strength{0.0f};
};

struct ObserveInput {
    AgentId agent{};
    Proposition proposition{};
    StimulusTime time{};
};

struct ObserveResult {
    bool state_changed{false};
    bool decision_recommended{false};
    Urgency urgency{Urgency::None};
    bool current_action_threatened{false};
};

struct ReportInput {
    AgentId agent{};
    ActionTypeId action_type{};
    bool success{false};
    PsychTime process_time{};
};

struct AdvanceResult {
    bool state_changed{false};
    bool decision_recommended{false};
};

struct ReactInput {
    ObserveInput observation{};
    std::vector<ActionOption> actions{};
    ActionContinuityKey current_action{};
};

struct ReactResult {
    ObserveResult observation{};
    Decision decision{};
};

struct ObserveBatchItem { std::uint64_t input_sequence{0}; ObserveInput input{}; };
struct ObserveBatchOutput { std::uint64_t input_sequence{0}; AgentId agent{}; ObserveResult result{}; };
struct DecideBatchItem { std::uint64_t input_sequence{0}; AgentId agent{}; std::vector<ActionOption> actions{}; PsychTime process_time{}; ActionContinuityKey current{}; };
struct DecideBatchOutput { std::uint64_t input_sequence{0}; AgentId agent{}; Decision result{}; };
struct ReportBatchItem { std::uint64_t input_sequence{0}; ReportInput input{}; };
struct ReportBatchOutput { std::uint64_t input_sequence{0}; AgentId agent{}; };
struct AdvanceBatchItem { std::uint64_t input_sequence{0}; AgentId agent{}; PsychTime target_time{}; };
struct AdvanceBatchOutput { std::uint64_t input_sequence{0}; AgentId agent{}; AdvanceResult result{}; };

class Runtime {
public:
    static Result<std::unique_ptr<Runtime>> create(RuntimeConfig config, RegistrySnapshot registry = {}) noexcept;

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(Runtime&&) = delete;
    ~Runtime() = default;

    Result<void> create_agent(AgentId id, const AgentInit& init, PsychTime process_time) noexcept;
    Result<void> create_agent_derived(AgentId id, const DerivedInit& derived, PsychTime process_time) noexcept;
    Result<void> remove_agent(AgentId id, PsychTime process_time,
                              RemovePolicy policy = RemovePolicy::ForgetRuntimeOnly) noexcept;
    [[nodiscard]] bool has_agent(AgentId id) const noexcept;

    Result<ObserveResult> observe(const ObserveInput& input) noexcept;
    Result<Decision> decide(AgentId agent, const std::vector<ActionOption>& actions,
                            PsychTime process_time, ActionContinuityKey current = {}) noexcept;
    Result<void> report(const ReportInput& input) noexcept;
    Result<AdvanceResult> advance_to(AgentId agent, PsychTime target_time) noexcept;
    Result<ReactResult> react(const ReactInput& input) noexcept;

    Result<std::vector<ObserveBatchOutput>> observe_batch(std::vector<ObserveBatchItem> items) noexcept;
    Result<std::vector<DecideBatchOutput>> decide_batch(std::vector<DecideBatchItem> items) noexcept;
    Result<std::vector<ReportBatchOutput>> report_batch(std::vector<ReportBatchItem> items) noexcept;
    Result<std::vector<AdvanceBatchOutput>> advance_batch(std::vector<AdvanceBatchItem> items) noexcept;

    Result<ExportedAgent> export_agent(AgentId id, PsychTime exported_at) const noexcept;
    Result<ImportResult> import_agent(const ExportedAgent& exported, const ImportOptions& options) noexcept;

    Result<void> begin_registry_staging() noexcept;
    Result<void> stage_domain_pack(const DomainPack& pack) noexcept;
    Result<void> publish_registry() noexcept;
    void discard_registry_staging() noexcept;
    [[nodiscard]] bool has_registry_staging() const noexcept;

    [[nodiscard]] const RuntimeConfig& config() const noexcept;
    [[nodiscard]] const RegistrySnapshot& registry() const noexcept;
    [[nodiscard]] const TraceSink& diagnostics() const noexcept;
    [[nodiscard]] const AgentStore& agents() const noexcept;

private:
    Runtime(RuntimeConfig config, RegistrySnapshot registry);

    RuntimeConfig config_{};
    RegistrySnapshot registry_{};
    std::unique_ptr<RegistryBuilder> registry_staging_{};
    AgentStore agents_{};
    PsychologyEngine psychology_{};
    UtilityComposer utility_{};
    TraceSink diagnostics_{};
};

} // namespace mspa
