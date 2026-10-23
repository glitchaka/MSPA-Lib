#include "mspa/runtime/runtime.hpp"

#include <algorithm>
#include <map>
#include <new>

namespace mspa {
namespace {

template <typename Item, typename AgentFn>
Result<void> canonicalize(std::vector<Item>& items, AgentFn agent_of) {
    std::stable_sort(items.begin(), items.end(), [&](const Item& a, const Item& b) {
        const AgentId aa = agent_of(a);
        const AgentId bb = agent_of(b);
        if (aa != bb) return aa < bb;
        return a.input_sequence < b.input_sequence;
    });
    for (std::size_t i = 1; i < items.size(); ++i) {
        if (agent_of(items[i - 1]) == agent_of(items[i]) &&
            items[i - 1].input_sequence == items[i].input_sequence) {
            return make_error(ErrorCode::BatchConflict, "duplicate input_sequence for one agent");
        }
    }
    return {};
}

float clamp01(float value) { return std::clamp(value, 0.0f, 1.0f); }

} // namespace

Runtime::Runtime(RuntimeConfig config, RegistrySnapshot registry)
    : config_(std::move(config)), registry_(std::move(registry)) {}

Result<std::unique_ptr<Runtime>> Runtime::create(RuntimeConfig config, RegistrySnapshot registry) noexcept {
    try {
        if (!config.runtime_id.valid() || !config.world_id.valid() || !config.time_domain.valid()) {
            return make_error(ErrorCode::InvalidArgument, "runtime id, world id and time domain must be valid");
        }
        auto limits = validate_limits(config.limits);
        if (!limits) return *limits.error_if();
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
        state.beliefs = init.beliefs;
        state.goals = init.goals;
        state.memories = init.memories;
        state.relationships = init.relationships;
        state.last_processed_time = process_time;
        if (init.use_custom_rng_state) {
            state.rng_state = init.rng_state;
        } else {
            DeterministicRng rng(derive_agent_seed(config_.seed_root, id, 0));
            state.rng_state = rng.state();
        }

        auto valid = validate_agent_state(state, config_.limits);
        if (!valid) return *valid.error_if();
        return agents_.insert(std::move(state));
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "agent creation allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception creating agent");
    }
}

Result<void> Runtime::create_agent_derived(AgentId id, const DerivedInit& derived, PsychTime process_time) noexcept {
    try {
        if (derived.source_agents.empty()) {
            return make_error(ErrorCode::InvalidArgument, "derived init requires at least one source agent");
        }
        std::vector<const AgentPsychState*> sources;
        for (const auto source_id : derived.source_agents) {
            const auto* source = agents_.find(source_id);
            if (!source) return make_error(ErrorCode::UnknownAgent, "derived init source agent does not exist");
            sources.push_back(source);
        }

        AgentInit init{};
        if (derived.trait_blend_policy == TraitBlendPolicy::FirstSource) {
            init.personality = sources.front()->personality;
        } else {
            Personality p{};
            p.sociability = p.stability = p.empathy = p.persistence = p.curiosity = p.boldness = 0.0f;
            for (const auto* source : sources) {
                p.sociability += source->personality.sociability;
                p.stability += source->personality.stability;
                p.empathy += source->personality.empathy;
                p.persistence += source->personality.persistence;
                p.curiosity += source->personality.curiosity;
                p.boldness += source->personality.boldness;
            }
            const float count = static_cast<float>(sources.size());
            p.sociability /= count;
            p.stability /= count;
            p.empathy /= count;
            p.persistence /= count;
            p.curiosity /= count;
            p.boldness /= count;
            init.personality = p;
        }

        if (derived.inherited_value_policy == InheritedValuePolicy::Mean) {
            std::map<ConceptId, std::pair<float, std::uint32_t>> values;
            for (const auto* source : sources) {
                for (const auto& value : source->values) {
                    auto& slot = values[value.concept_id];
                    slot.first += value.intensity;
                    ++slot.second;
                }
            }
            for (const auto& entry : values) {
                init.values.push_back({entry.first, entry.second.first / static_cast<float>(entry.second.second)});
            }
        }

        if (derived.variation_strength > 0.0f) {
            DeterministicRng rng(derive_agent_seed(derived.variation_seed, id, 77));
            auto vary = [&](float& value) {
                const float unit = static_cast<float>(rng.next_u32()) / static_cast<float>(UINT32_MAX);
                const float delta = (unit * 2.0f - 1.0f) * derived.variation_strength;
                value = clamp01(value + delta);
            };
            vary(init.personality.sociability);
            vary(init.personality.stability);
            vary(init.personality.empathy);
            vary(init.personality.persistence);
            vary(init.personality.curiosity);
            vary(init.personality.boldness);
        }
        return create_agent(id, init, process_time);
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "derived agent creation allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception creating derived agent");
    }
}

Result<void> Runtime::remove_agent(AgentId id, PsychTime process_time, RemovePolicy policy) noexcept {
    try {
        auto* state = agents_.find(id);
        if (!state) return make_error(ErrorCode::UnknownAgent, "agent does not exist");
        if (process_time < state->last_processed_time) {
            return make_error(ErrorCode::TimeRegression, "remove process_time regressed");
        }
        if (policy == RemovePolicy::ValidateNoReferences) {
            return make_error(ErrorCode::UnsupportedOperation, "reference graph validation is not implemented yet");
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
        if (state->beliefs.size() >= config_.limits.max_beliefs_per_agent) {
            return make_error(ErrorCode::QuotaExceeded, "belief limit reached for agent");
        }

        AppraisalResult appraisal{};
        auto changed = psychology_.observe(*state, input.proposition, input.time.process_time,
                                           input.time.event_time, input.time.has_event_time,
                                           registry_, &appraisal);
        if (!changed) return *changed.error_if();

        ObserveResult result{};
        result.state_changed = *changed.value_if();
        result.decision_recommended = appraisal.relevance >= 0.5f;
        result.urgency = appraisal.relevance >= 0.95f ? Urgency::Immediate :
                         appraisal.relevance >= 0.75f ? Urgency::Soon :
                         appraisal.relevance > 0.0f ? Urgency::Normal : Urgency::None;
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
                                 PsychTime process_time, ActionContinuityKey current,
                                 const DecisionContext& context) noexcept {
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
            auto score = utility_.score(action, *state, context);
            const float continuity = hysteresis_.continuity_bonus(*state, action, current);
            if (continuity != 0.0f) {
                score.considerations.push_back({"continuity/hysteresis", action.progress, continuity});
                score.utility += continuity;
            }
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
        state->current_intent = ActionContinuityKey{};
        if (decision.has_selection) {
            const auto it = std::find_if(actions.begin(), actions.end(),
                                         [&](const ActionOption& action) {
                                             return action.instance == decision.selected_action;
                                         });
            if (it != actions.end()) state->current_intent = it->continuity;
        }
        decision.confidence = decision.has_selection ? clamp01(0.5f + std::max(0.0f, best) * 0.1f) : 0.0f;

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
                               [&](const LearnedExpectation& expectation) {
                                   return expectation.action == input.action_type;
                               });
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

Result<ReactResult> Runtime::react(const ReactInput& input) noexcept {
    auto observed = observe(input.observation);
    if (!observed) return *observed.error_if();
    auto decision = decide(input.observation.agent, input.actions,
                           input.observation.time.process_time, input.current_action,
                           input.decision_context);
    if (!decision) return *decision.error_if();
    return ReactResult{*observed.value_if(), *decision.value_if()};
}

Result<std::vector<ObserveBatchOutput>> Runtime::observe_batch(std::vector<ObserveBatchItem> items) noexcept {
    auto ordered = canonicalize(items, [](const ObserveBatchItem& item) { return item.input.agent; });
    if (!ordered) return *ordered.error_if();
    std::vector<ObserveBatchOutput> out;
    out.reserve(items.size());
    for (const auto& item : items) {
        auto result = observe(item.input);
        if (!result) return *result.error_if();
        out.push_back({item.input_sequence, item.input.agent, *result.value_if()});
    }
    return out;
}

Result<std::vector<DecideBatchOutput>> Runtime::decide_batch(std::vector<DecideBatchItem> items) noexcept {
    auto ordered = canonicalize(items, [](const DecideBatchItem& item) { return item.agent; });
    if (!ordered) return *ordered.error_if();
    std::vector<DecideBatchOutput> out;
    out.reserve(items.size());
    for (const auto& item : items) {
        auto result = decide(item.agent, item.actions, item.process_time, item.current, item.context);
        if (!result) return *result.error_if();
        out.push_back({item.input_sequence, item.agent, *result.value_if()});
    }
    return out;
}

Result<std::vector<ReportBatchOutput>> Runtime::report_batch(std::vector<ReportBatchItem> items) noexcept {
    auto ordered = canonicalize(items, [](const ReportBatchItem& item) { return item.input.agent; });
    if (!ordered) return *ordered.error_if();
    std::vector<ReportBatchOutput> out;
    out.reserve(items.size());
    for (const auto& item : items) {
        auto result = report(item.input);
        if (!result) return *result.error_if();
        out.push_back({item.input_sequence, item.input.agent});
    }
    return out;
}

Result<std::vector<AdvanceBatchOutput>> Runtime::advance_batch(std::vector<AdvanceBatchItem> items) noexcept {
    auto ordered = canonicalize(items, [](const AdvanceBatchItem& item) { return item.agent; });
    if (!ordered) return *ordered.error_if();
    std::vector<AdvanceBatchOutput> out;
    out.reserve(items.size());
    for (const auto& item : items) {
        auto result = advance_to(item.agent, item.target_time);
        if (!result) return *result.error_if();
        out.push_back({item.input_sequence, item.agent, *result.value_if()});
    }
    return out;
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

Result<void> Runtime::begin_registry_staging() noexcept {
    try {
        registry_staging_ = std::make_unique<RegistryBuilder>(registry_);
        return {};
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "registry staging allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception staging registry");
    }
}

Result<void> Runtime::stage_domain_pack(const DomainPack& pack) noexcept {
    try {
        if (!registry_staging_) {
            auto started = begin_registry_staging();
            if (!started) return *started.error_if();
        }
        return apply_domain_pack(*registry_staging_, pack);
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "domain pack staging allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception staging domain pack");
    }
}

Result<void> Runtime::publish_registry() noexcept {
    try {
        if (!registry_staging_) return make_error(ErrorCode::InvalidState, "no registry staging exists");
        registry_ = registry_staging_->publish();
        registry_staging_.reset();
        return {};
    } catch (const std::bad_alloc&) {
        return make_error(ErrorCode::OutOfMemory, "registry publication allocation failed");
    } catch (...) {
        return make_error(ErrorCode::InternalInvariantViolation, "unexpected exception publishing registry");
    }
}

void Runtime::discard_registry_staging() noexcept {
    registry_staging_.reset();
}

bool Runtime::has_registry_staging() const noexcept {
    return static_cast<bool>(registry_staging_);
}

const RuntimeConfig& Runtime::config() const noexcept { return config_; }
const RegistrySnapshot& Runtime::registry() const noexcept { return registry_; }
const TraceSink& Runtime::diagnostics() const noexcept { return diagnostics_; }
const AgentStore& Runtime::agents() const noexcept { return agents_; }

} // namespace mspa
