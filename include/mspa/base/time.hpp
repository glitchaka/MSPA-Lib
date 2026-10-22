#pragma once

#include <cstdint>

#include "mspa/base/ids.hpp"

namespace mspa {

class PsychTime {
public:
    constexpr PsychTime() noexcept = default;
    explicit constexpr PsychTime(std::uint64_t ticks) noexcept : ticks_(ticks) {}

    [[nodiscard]] constexpr std::uint64_t ticks() const noexcept { return ticks_; }

    friend constexpr bool operator==(PsychTime lhs, PsychTime rhs) noexcept {
        return lhs.ticks_ == rhs.ticks_;
    }

    friend constexpr bool operator!=(PsychTime lhs, PsychTime rhs) noexcept {
        return !(lhs == rhs);
    }

    friend constexpr bool operator<(PsychTime lhs, PsychTime rhs) noexcept {
        return lhs.ticks_ < rhs.ticks_;
    }

    friend constexpr bool operator<=(PsychTime lhs, PsychTime rhs) noexcept {
        return lhs.ticks_ <= rhs.ticks_;
    }

    friend constexpr bool operator>(PsychTime lhs, PsychTime rhs) noexcept {
        return rhs < lhs;
    }

    friend constexpr bool operator>=(PsychTime lhs, PsychTime rhs) noexcept {
        return rhs <= lhs;
    }

private:
    std::uint64_t ticks_{};
};

struct TimeScale {
    std::uint64_t ticks_per_second_num{1};
    std::uint64_t ticks_per_second_den{1};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return ticks_per_second_num != 0 && ticks_per_second_den != 0;
    }
};

struct TimeDomain {
    TimeDomainId id{};
    TimeScale scale{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return id.valid() && scale.valid();
    }
};

enum class ImportTimePolicy : std::uint8_t {
    RebaseRelative,
    PreserveAbsolute,
    RejectIncompatible
};

struct StimulusTime {
    PsychTime process_time{};
    bool has_event_time{false};
    PsychTime event_time{};
};

} // namespace mspa
