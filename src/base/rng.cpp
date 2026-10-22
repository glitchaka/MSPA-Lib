#include "mspa/base/rng.hpp"

namespace mspa {
namespace {

constexpr std::uint64_t kGoldenGamma = 0x9E3779B97F4A7C15ULL;

std::uint64_t mix64(std::uint64_t value) noexcept {
    value += kGoldenGamma;
    value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
    value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;
    return value ^ (value >> 31U);
}

} // namespace

std::uint64_t derive_agent_seed(
    std::uint64_t seed_root,
    AgentId agent,
    std::uint64_t stream_kind) noexcept {
    std::uint64_t seed = mix64(seed_root);
    seed ^= mix64(agent.world.value());
    seed ^= mix64(agent.local_id);
    seed ^= mix64(stream_kind);
    return mix64(seed);
}

DeterministicRng::DeterministicRng(std::uint64_t seed) noexcept
    : state_(seed) {}

DeterministicRng::DeterministicRng(RngState state) noexcept
    : state_(state.state) {}

std::uint64_t DeterministicRng::next_u64() noexcept {
    state_ += kGoldenGamma;
    std::uint64_t value = state_;
    value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
    value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;
    return value ^ (value >> 31U);
}

std::uint32_t DeterministicRng::next_u32() noexcept {
    return static_cast<std::uint32_t>(next_u64() >> 32U);
}

RngState DeterministicRng::state() const noexcept {
    return RngState{state_};
}

} // namespace mspa
