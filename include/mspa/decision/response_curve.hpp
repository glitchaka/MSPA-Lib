#pragma once

#include <cstddef>
#include <vector>

#include "mspa/base/result.hpp"

namespace mspa {

struct CurvePoint {
    float x{0.0f};
    float y{0.0f};
};

class ResponseCurve {
public:
    Result<void> set_points(std::vector<CurvePoint> points);
    void clear() noexcept;
    [[nodiscard]] float evaluate(float input) const noexcept;
    [[nodiscard]] const std::vector<CurvePoint>& points() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::vector<CurvePoint> points_{};
};

} // namespace mspa
