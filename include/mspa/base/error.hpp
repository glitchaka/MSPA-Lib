#pragma once

#include <cstdint>
#include <string>

namespace mspa {

enum class ErrorCode : std::uint16_t {
    None = 0,
    InvalidArgument,
    UnknownAgent,
    DuplicateAgent,
    InvalidState,
    InvalidPack,
    QuotaExceeded,
    VersionMismatch,
    NamespaceConflict,
    TimeRegression,
    BatchConflict,
    OutOfMemory,
    UnsupportedOperation,
    InternalInvariantViolation
};

struct Error {
    ErrorCode code{ErrorCode::None};
    std::string details{};

    [[nodiscard]] bool ok() const noexcept {
        return code == ErrorCode::None;
    }
};

} // namespace mspa
