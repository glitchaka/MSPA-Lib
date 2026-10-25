#pragma once
#include <cstdint>
namespace mspa {
// 1.1 adds host-defined, extensible psychological profiles without core presets.
inline constexpr std::uint32_t runtime_version_major = 1;
inline constexpr std::uint32_t runtime_version_minor = 1;
inline constexpr std::uint32_t runtime_version_patch = 0;
inline constexpr std::uint32_t state_schema_version = 1;
inline constexpr std::uint32_t serialization_format_version = 1;
}
