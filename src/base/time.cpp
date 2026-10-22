#include "mspa/base/time.hpp"

#include <limits>
#include <numeric>

namespace mspa {

namespace {

bool checked_mul(std::uint64_t a, std::uint64_t b, std::uint64_t& out) noexcept {
    if (a == 0 || b == 0) {
        out = 0;
        return true;
    }
    if (a > std::numeric_limits<std::uint64_t>::max() / b) {
        return false;
    }
    out = a * b;
    return true;
}

} // namespace

Result<PsychTime> convert_time_delta(PsychTime delta, TimeScale source, TimeScale destination) {
    if (!source.valid() || !destination.valid()) {
        return make_error(ErrorCode::InvalidArgument, "time scales must be valid");
    }

    std::uint64_t a = delta.ticks();
    std::uint64_t b = source.ticks_per_second_den;
    std::uint64_t c = destination.ticks_per_second_num;
    std::uint64_t d = source.ticks_per_second_num;
    std::uint64_t e = destination.ticks_per_second_den;

    auto reduce = [](std::uint64_t& x, std::uint64_t& y) {
        const auto g = std::gcd(x, y);
        x /= g;
        y /= g;
    };

    reduce(a, d);
    reduce(a, e);
    reduce(b, d);
    reduce(b, e);
    reduce(c, d);
    reduce(c, e);

    std::uint64_t numerator = 0;
    std::uint64_t temp = 0;
    if (!checked_mul(a, b, temp) || !checked_mul(temp, c, numerator)) {
        return make_error(ErrorCode::InvalidState, "time conversion overflow");
    }

    std::uint64_t denominator = 0;
    if (!checked_mul(d, e, denominator) || denominator == 0) {
        return make_error(ErrorCode::InvalidState, "time conversion denominator overflow");
    }

    return PsychTime(numerator / denominator);
}

} // namespace mspa
