#include "mspa/runtime/runtime.hpp"

#include <algorithm>
#include <exception>
#include <new>

namespace mspa {

Runtime::Runtime(RuntimeConfig config, RegistrySnapshot registry)
    : config_(std::move(config)), registry_(std::move(registry)) {}

Result<std::unique_ptr<Runtime>> Runtime::create(RuntimeConfig config, RegistrySnapshot registry) noexcept {
    try {
        if (!config.runtime_id.valid() || !config.world_id.valid() || !config.time_domain.valid()) {
            return make_error(ErrorCode::InvalidArgument, "runtime id, world id and time domain must be valid");
        }
        auto limits = validate_limits(config.limits);
        if (!limits) {
            return *limits.error_if();
        }
        return std::unique_ptr<Runtime>(new Runtime(std::move(config), std::move(registry)));
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "runtime allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception creating runtime");
    }
}

Result<void> Runtime::create_agent(AgentId id, const AgentInit& init, PsychTime process_time) noexcept {
    try {
        if (!id.valid() || id.world != config_.world_id) {
            return make_error(ErrorCode::InvalidArgument, "agent id must belong to runtime world");
        }
        if (agents_.size() >= config_.limits.max_agents) {
            return make_error(ErrorCode::QuotaExceeded, "runtime agent limit reached");
        }

        AgentPsychState state{};
        state.id = id;
        state.personality = init.personality;
        state.values = init.values;
        state.last_processed_time = process_time;
        DeterministicRng rng(derive_agent_seed(config_.seed_root, id, 0));
        state.rng_state = rng.state();

        auto valid = validate_agent_state(state, config_.limits);
        if (!valid) return *valid.error_if();
        return agents_.insert(std::move(state));
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "agent creation allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception creating agent");
    }
}

Result<void> Runtime::remove_agent(AgentId id, PsychTime process_time) noexcept {
    try {
        auto* state = agents_.find(id);
        if (!state) return make_error(ErrorCode::UnknownAgent, "agent does not exist");
        if (process_time < state->last_processed_time) {
            return make_error(ErrorCode::TimeRegression, "remove process_time regressed");
        }
        return agents_.erase(id);
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "agent removal allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception removing agent");
    }
}

bool Runtime::has_agent(AgentId id) const noexcept {
    return agents_.contains(id);
}

Result<ObserveResult> Runtime::observe(const ObserveInput& input) noexcept {
    try {
        auto* state = agents_.find(input.agent);
        if (!state) return make_error(ErrorCode::UnknownAgent, "agent does not exist");

        AppraisalResult appraisal{};
        auto changed = psychology_.observe(*state, input.proposition, input.time.process_time,
                                           input.time.event_time, input.time.has_event_time,
                                           registry_, &appraisal);
        if (!changed) return *changed.error_if();

        ObserveResult result{};
        result.state_changed = *changed.value_if();
        result.decision_recommended = appraisal.relevance >= 0.5f;
        result.urgency = appraisal.relevance >= 0.9f ? Urgency::Soon : Urgency::Normal;
        result.current_action_threatened = appraisal.consequence < -0.5f;

        if (config_.diagnostics_enabled) {
            DiagnosticTrace trace{};
            trace.agent = input.agent;
            trace.process_time = input.time.process_time;
            trace.has_event_time = input.time.has_event_time;
            trace.event_time = input.time.event_time;
            trace.items.push_back({"observe", "proposition processed", appraisal.relevance});
            trace.items.push_back({"appraisal", "consequence", appraisal.consequence});
            diagnostics_.push(std::move(trace));
        }
        return result;
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "observe allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception during observe");
    }
}

Result<Decision> Runtime::decide(AgentId agent, const std::vector<ActionOption>& actions,
                                 PsychTime process_time, ActionContinuityKey) noexcept {
    try {
        auto* state = agents_.find(agent);
        if (!state) return make_error(ErrorCode::UnknownAgent, "agent does not exist");
        if (process_time < state->last_processed_time) {
            return make_error(ErrorCode::TimeRegression, "decide process_time regressed");
        }

        Decision decision{};
        float best = 0.0f;
        bool first = true;
        for (const auto& action : actions) {
            if (action.actor != agent) {
                return make_error(ErrorCode::InvalidArgument, "action actor does not match deciding agent");
            }
            auto score = utility_.score(action);
            decision.trace.push_back(score);
            if (first || score.utility > best) {
                first = false;
                best = score.utility;
                decision.has_selection = true;
                decision.selected_action = action.instance;
                decision.selected_target = action.targets.empty() ? AgentId{} : action.targets.front();
            }
        }
        state->last_processed_time = process_time;
        state->current_intent = decision.has_selection ?
            std::find_if(actions.begin(), actions.end(), [&](const ActionOption& a) { return a.instance == decision.selected_action; })->continuity : ActionContinuityKey{};
        decision.confidence = decision.has_selection ? 1.0f : 0.0f;

        if (config_.diagnostics_enabled) {
            DiagnosticTrace trace{};
            trace.agent = agent;
            trace.process_time = process_time;
            trace.items.push_back({"decide", decision.has_selection ? "action selected" : "no action available", best});
            diagnostics_.push(std::move(trace));
        }
        return decision;
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "decide allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception during decide");
    }
}

Result<void> Runtime::report(const ReportInput& input) noexcept {
    try {
        auto* state = agents_.find(input.agent);
        if (!state) return make_error(ErrorCode::UnknownAgent, "agent does not exist");
        if (input.process_time < state->last_processed_time) {
            return make_error(ErrorCode::TimeRegression, "report process_time regressed");
        }

        auto it = std::find_if(state->learned_expectations.begin(), state->learned_expectations.end(),
                               [&](const LearnedExpectation& e) { return e.action == input.action_type; });
        if (it == state->learned_expectations.end()) {
            state->learned_expectations.push_back({input.action_type, 1u, input.success ? 1u : 0u, 0.5f});
        } else {
            ++it->attempts;
            if (input.success) ++it->successes;
            it->confidence = static_cast<float>(it->successes) / static_cast<float>(it->attempts);
        }
        state->last_processed_time = input.process_time;
        return {};
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "report allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception during report");
    }
}

Result<AdvanceResult> Runtime::advance_to(AgentId agent, PsychTime target_time) noexcept {
    try {
        auto* state = agents_.find(agent);
        if (!state) return make_error(ErrorCode::UnknownAgent, "agent does not exist");
        if (target_time < state->last_processed_time) {
            return make_error(ErrorCode::TimeRegression, "advance target_time regressed");
        }
        const bool changed = target_time != state->last_processed_time;
        state->last_processed_time = target_time;
        return AdvanceResult{changed, false};
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "advance allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception during advance");
    }
}

Result<ExportedAgent> Runtime::export_agent(AgentId id, PsychTime exported_at) const noexcept {
    try {
        const auto* state = agents_.find(id);
        if (!state) return make_error(ErrorCode::UnknownAgent, "agent does not exist");
        if (exported_at < state->last_processed_time) {
            return make_error(ErrorCode::TimeRegression, "exported_at precedes agent state");
        }
        return ExportedAgent{*state, config_.time_domain, exported_at};
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "export allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception during export");
    }
}

Result<ImportResult> Runtime::import_agent(const ExportedAgent& exported, const ImportOptions& options) noexcept {
    try {
        if (options.destination_id.world != config_.world_id) {
            return make_error(ErrorCode::InvalidArgument, "destination agent must belong to runtime world");
        }
        if (agents_.size() >= config_.limits.max_agents) {
            return make_error(ErrorCode::QuotaExceeded, "runtime agent limit reached");
        }
        auto imported = import_agent_state(exported, config_.time_domain, options);
        if (!imported) return *imported.error_if();
        auto valid = validate_agent_state(imported.value_if()->state, config_.limits);
        if (!valid) return *valid.error_if();
        auto inserted = agents_.insert(imported.value_if()->state);
        if (!inserted) return *inserted.error_if();
        return std::move(*imported.value_if());
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "import allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception during import");
    }
}

const RuntimeConfig& Runtime::config() const noexcept { return config_; }
const RegistrySnapshot& Runtime::registry() const noexcept { return registry_; }
const TraceSink& Runtime::diagnostics() const noexcept { return diagnostics_; }
const AgentStore& Runtime::agents() const noexcept { return agents_; }

} // namespace mspa
