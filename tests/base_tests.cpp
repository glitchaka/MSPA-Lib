#include <cassert>
#include <cstdint>
#include <iostream>
#include <type_traits>

#include "mspa/base/error.hpp"
#include "mspa/base/ids.hpp"
#include "mspa/base/result.hpp"
#include "mspa/base/rng.hpp"
#include "mspa/base/time.hpp"

namespace {

void test_strong_ids_and_agent_identity() {
    static_assert(!std::is_convertible_v<mspa::WorldId, mspa::RuntimeId>);

    const mspa::WorldId world_a{1};
    const mspa::WorldId world_b{2};
    const mspa::AgentId a1{world_a, 7};
    const mspa::AgentId a2{world_a, 7};
    const mspa::AgentId b1{world_b, 7};

    assert(a1 == a2);
    assert(a1 != b1);
    assert(a1.valid());
    assert(b1.valid());
}

void test_time_types() {
    const mspa::TimeDomain domain{mspa::TimeDomainId{5}, mspa::TimeScale{60, 1}};
    assert(domain.valid());

    const mspa::StimulusTime delayed{
        mspa::PsychTime{120},
        true,
        mspa::PsychTime{30}
    };

    assert(delayed.event_time < delayed.process_time);
    assert(delayed.has_event_time);
}

void test_result_contract() {
    mspa::Result<int> ok{42};
    assert(ok.has_value());
    assert(ok.value_if() != nullptr);
    assert(*ok.value_if() == 42);
    assert(ok.error_if() == nullptr);

    mspa::Result<int> failure{mspa::make_error(mspa::ErrorCode::OutOfMemory, "allocation failed")};
    assert(!failure.has_value());
    assert(failure.value_if() == nullptr);
    assert(failure.error_if() != nullptr);
    assert(failure.error_if()->code == mspa::ErrorCode::OutOfMemory);
}

void test_rng_determinism_and_isolation() {
    constexpr std::uint64_t root_seed = 0x123456789ABCDEF0ULL;

    const mspa::AgentId agent_a{mspa::WorldId{10}, 99};
    const mspa::AgentId same_local_other_world{mspa::WorldId{11}, 99};

    const auto seed_a = mspa::derive_agent_seed(root_seed, agent_a, 0);
    const auto seed_a_again = mspa::derive_agent_seed(root_seed, agent_a, 0);
    const auto seed_other_world = mspa::derive_agent_seed(root_seed, same_local_other_world, 0);
    const auto seed_other_stream = mspa::derive_agent_seed(root_seed, agent_a, 1);

    assert(seed_a == seed_a_again);
    assert(seed_a != seed_other_world);
    assert(seed_a != seed_other_stream);

    mspa::DeterministicRng first{seed_a};
    mspa::DeterministicRng second{seed_a};

    for (int i = 0; i < 16; ++i) {
        assert(first.next_u64() == second.next_u64());
    }

    mspa::DeterministicRng original{seed_a};
    (void)original.next_u64();
    const mspa::RngState saved = original.state();
    const auto expected_next = original.next_u64();

    mspa::DeterministicRng restored{saved};
    assert(restored.next_u64() == expected_next);
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
