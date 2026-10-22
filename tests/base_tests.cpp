#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <type_traits>

#include "mspa/base/error.hpp"
#include "mspa/base/ids.hpp"
#include "mspa/base/result.hpp"
#include "mspa/base/rng.hpp"
#include "mspa/base/time.hpp"

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

void test_strong_ids_and_agent_identity() {
    static_assert(!std::is_convertible_v<mspa::WorldId, mspa::RuntimeId>);
    const mspa::WorldId world_a{1};
    const mspa::WorldId world_b{2};
    const mspa::AgentId a1{world_a, 7};
    const mspa::AgentId a2{world_a, 7};
    const mspa::AgentId b1{world_b, 7};
    require(a1 == a2, "same world/local id must compare equal");
    require(a1 != b1, "same local id in different worlds must differ");
    require(a1.valid() && b1.valid(), "agent ids must be valid");
}

void test_time_types() {
    const mspa::TimeDomain domain{mspa::TimeDomainId{5}, mspa::TimeScale{60, 1}};
    require(domain.valid(), "time domain must be valid");
    const mspa::StimulusTime delayed{mspa::PsychTime{120}, true, mspa::PsychTime{30}};
    require(delayed.event_time < delayed.process_time, "late event must be representable");
    require(delayed.has_event_time, "event time flag must be retained");

    auto converted = mspa::convert_time_delta(mspa::PsychTime{60}, mspa::TimeScale{60, 1}, mspa::TimeScale{120, 1});
    require(static_cast<bool>(converted), "time delta conversion must succeed");
    require(converted.value_if()->ticks() == 120, "time delta conversion must preserve duration");
}

void test_result_contract() {
    mspa::Result<int> ok{42};
    require(ok.has_value(), "successful Result must have value");
    require(ok.value_if() && *ok.value_if() == 42, "successful Result must expose value");
    require(ok.error_if() == nullptr, "successful Result must not expose error");

    mspa::Result<int> failure{mspa::make_error(mspa::ErrorCode::OutOfMemory, "allocation failed")};
    require(!failure.has_value(), "failed Result must not have value");
    require(failure.value_if() == nullptr, "failed Result value must be null");
    require(failure.error_if() && failure.error_if()->code == mspa::ErrorCode::OutOfMemory,
            "failed Result must expose error code");
}

void test_rng_determinism_and_isolation() {
    constexpr std::uint64_t root_seed = 0x123456789ABCDEF0ULL;
    const mspa::AgentId agent_a{mspa::WorldId{10}, 99};
    const mspa::AgentId same_local_other_world{mspa::WorldId{11}, 99};

    const auto seed_a = mspa::derive_agent_seed(root_seed, agent_a, 0);
    const auto seed_a_again = mspa::derive_agent_seed(root_seed, agent_a, 0);
    const auto seed_other_world = mspa::derive_agent_seed(root_seed, same_local_other_world, 0);
    const auto seed_other_stream = mspa::derive_agent_seed(root_seed, agent_a, 1);
    require(seed_a == seed_a_again, "same agent/stream must reproduce seed");
    require(seed_a != seed_other_world, "world must affect seed");
    require(seed_a != seed_other_stream, "stream kind must affect seed");

    mspa::DeterministicRng first{seed_a};
    mspa::DeterministicRng second{seed_a};
    for (int i = 0; i < 16; ++i) require(first.next_u64() == second.next_u64(), "RNG sequence must be deterministic");

    mspa::DeterministicRng original{seed_a};
    (void)original.next_u64();
    const mspa::RngState saved = original.state();
    const auto expected_next = original.next_u64();
    mspa::DeterministicRng restored{saved};
    require(restored.next_u64() == expected_next, "restored RNG state must resume sequence");
}

} // namespace

int main() {
    test_strong_ids_and_agent_identity();
    test_time_types();
    test_result_contract();
    test_rng_determinism_and_isolation();
    std::cout << "MSPA base tests passed\n";
    return 0;
}
