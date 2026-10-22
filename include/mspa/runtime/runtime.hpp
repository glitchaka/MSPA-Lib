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
#include "mspa/semantics/registry.hpp"
#include "mspa/validation/limits.hpp"

namespace mspa {

enum class Urgency : std::uint8_t {
    None,
    Normal,
    Soon,
    Immediate
};

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

class Runtime {
public:
    static Result<std::unique_ptr<Runtime>> create(RuntimeConfig config, RegistrySnapshot registry = {}) noexcept;

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(Runtime&&) = delete;
    ~Runtime() = default;

    Result<void> create_agent(AgentId id, const AgentInit& init, PsychTime process_time) noexcept;
    Result<void> remove_agent(AgentId id, PsychTime process_time) noexcept;
    [[nodiscard]] bool has_agent(AgentId id) const noexcept;

    Result<ObserveResult> observe(const ObserveInput& input) noexcept;
    Result<Decision> decide(AgentId agent, const std::vector<ActionOption>& actions,
                            PsychTime process_time, ActionContinuityKey current = {}) noexcept;
    Result<void> report(const ReportInput& input) noexcept;
    Result<AdvanceResult> advance_to(AgentId agent, PsychTime target_time) noexcept;

    Result<ExportedAgent> export_agent(AgentId id, PsychTime exported_at) const noexcept;
    Result<ImportResult> import_agent(const ExportedAgent& exported, const ImportOptions& options) noexcept;

    [[nodiscard]] const RuntimeConfig& config() const noexcept;
    [[nodiscard]] const RegistrySnapshot& registry() const noexcept;
    [[nodiscard]] const TraceSink& diagnostics() const noexcept;
    [[nodiscard]] const AgentStore& agents() const noexcept;

private:
    Runtime(RuntimeConfig config, RegistrySnapshot registry);

    template <typename Fn>
    auto guard(Fn&& fn) noexcept -> decltype(fn());

    RuntimeConfig config_{};
    RegistrySnapshot registry_{};
    AgentStore agents_{};
    PsychologyEngine psychology_{};
    UtilityComposer utility_{};
    TraceSink diagnostics_{};
};

} // namespace mspa
