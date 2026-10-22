#include "mspa/persistence/state_transfer.hpp"

#include "mspa/base/rng.hpp"

namespace mspa {

namespace {

Result<PsychTime> rebase_timestamp(PsychTime stamp, PsychTime exported_at, PsychTime import_time,
                                   TimeScale source, TimeScale destination) {
    if (stamp > exported_at) {
        return make_error(ErrorCode::InvalidState, "exported state contains future timestamp");
    }
    const PsychTime age(exported_at.ticks() - stamp.ticks());
    auto converted = convert_time_delta(age, source, destination);
    if (!converted) {
        return *converted.error_if();
    }
    const auto converted_ticks = converted.value_if()->ticks();
    if (converted_ticks > import_time.ticks()) {
        return PsychTime(0);
    }
    return PsychTime(import_time.ticks() - converted_ticks);
}

} // namespace

Result<ImportResult> import_agent_state(
    const ExportedAgent& exported,
    const TimeDomain& destination_time_domain,
    const ImportOptions& options) {

    if (!exported.state.id.valid() || !exported.origin_time_domain.valid() ||
        !destination_time_domain.valid() || !options.destination_id.valid()) {
        return make_error(ErrorCode::InvalidArgument, "invalid state, time domain or destination id");
    }

    ImportResult result{};
    result.original_id = exported.state.id;
    result.effective_id = options.destination_id;
    result.time_policy = options.time_policy;
    result.rng_policy = options.rng_policy;
    result.state = exported.state;
    result.state.id = options.destination_id;

    if (options.time_policy == ImportTimePolicy::PreserveAbsolute) {
        if (exported.origin_time_domain.id != destination_time_domain.id) {
            return make_error(ErrorCode::VersionMismatch, "absolute time preservation requires the same time domain");
        }
    } else if (options.time_policy == ImportTimePolicy::RejectIncompatible) {
        if (exported.origin_time_domain.id != destination_time_domain.id ||
            exported.origin_time_domain.scale.ticks_per_second_num != destination_time_domain.scale.ticks_per_second_num ||
            exported.origin_time_domain.scale.ticks_per_second_den != destination_time_domain.scale.ticks_per_second_den) {
            return make_error(ErrorCode::VersionMismatch, "time domains are incompatible");
        }
    } else {
        auto last = rebase_timestamp(result.state.last_processed_time, exported.exported_at, options.import_time,
                                     exported.origin_time_domain.scale, destination_time_domain.scale);
        if (!last) return *last.error_if();
        result.state.last_processed_time = *last.value_if();

        for (auto& belief : result.state.beliefs) {
            auto t = rebase_timestamp(belief.updated_at, exported.exported_at, options.import_time,
                                      exported.origin_time_domain.scale, destination_time_domain.scale);
            if (!t) return *t.error_if();
            belief.updated_at = *t.value_if();
        }
        for (auto& memory : result.state.memories) {
            auto t = rebase_timestamp(memory.event_time, exported.exported_at, options.import_time,
                                      exported.origin_time_domain.scale, destination_time_domain.scale);
            if (!t) return *t.error_if();
            memory.event_time = *t.value_if();
        }
    }

    if (options.rng_policy == RngImportPolicy::RekeyForDestination) {
        AgentRng rng = AgentRng::from_seed(options.destination_seed_root, options.destination_id, 0);
        result.state.rng_state = rng.state();
    }

    return result;
}

} // namespace mspa
