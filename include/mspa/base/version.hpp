#pragma once
#include <cstdint>
namespace mspa {
// 1.1.3 adds explicit descendant creation with attenuated inheritance of
// learned action expectations while keeping autobiographical state personal.
inline constexpr std::uint32_t runtime_version_major = 1;
inline constexpr std::uint32_t runtime_version_minor = 1;
inline constexpr std::uint32_t runtime_version_patch = 3;
inline constexpr std::uint32_t state_schema_version = 1;
inline constexpr std::uint32_t serialization_format_version = 1;
}
