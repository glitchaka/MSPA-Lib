#pragma once
#include <cstdint>
namespace mspa {
// 1.1.2 hardens behavioral conformance: contextual success/failure learning,
// tonic-vs-episodic emotion handling, explainable hysteresis, and audit suites.
inline constexpr std::uint32_t runtime_version_major = 1;
inline constexpr std::uint32_t runtime_version_minor = 1;
inline constexpr std::uint32_t runtime_version_patch = 2;
inline constexpr std::uint32_t state_schema_version = 1;
inline constexpr std::uint32_t serialization_format_version = 1;
}
