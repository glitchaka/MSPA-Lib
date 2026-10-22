#pragma once

#include <cstdint>

#include "mspa/base/ids.hpp"

namespace mspa {

enum class RngImportPolicy : std::uint8_t {
    RekeyForDestination,
    PreserveState
};

struct RngState {
    std::uint64_t state{};
};

[[nodiscard]] std::uint64_t derive_agent_seed(
    std::uint64_t seed_root,
    AgentId agent,
    std::uint64_t stream_kind = 0) noexcept;

class DeterministicRng {
public:
    explicit DeterministicRng(std::uint64_t seed) noexcept;
    explicit DeterministicRng(RngState state) noexcept;

    [[nodiscard]] std::uint64_t next_u64() noexcept;
    [[nodiscard]] std::uint32_t next_u32() noexcept;
    [[nodiscard]] RngState state() const noexcept;

private:
    std::uint64_t state_{};
};

} // namespace mspa
