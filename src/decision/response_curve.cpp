#include "mspa/decision/response_curve.hpp"

#include <algorithm>

namespace mspa {

Result<void> ResponseCurve::set_points(std::vector<CurvePoint> points) {
    if (points.empty()) return make_error(ErrorCode::InvalidArgument, "response curve requires at least one point");
    if (!std::is_sorted(points.begin(), points.end(), [](const CurvePoint& a, const CurvePoint& b) { return a.x < b.x; })) {
        return make_error(ErrorCode::InvalidArgument, "response curve x values must be sorted");
    }
    points_ = std::move(points);
    return {};
}

void ResponseCurve::clear() noexcept { points_.clear(); }

float ResponseCurve::evaluate(float input) const noexcept {
    if (points_.empty()) return input;
    if (input <= points_.front().x) return points_.front().y;
    if (input >= points_.back().x) return points_.back().y;
    for (std::size_t i = 1; i < points_.size(); ++i) {
        if (input <= points_[i].x) {
            const auto& a = points_[i - 1];
            const auto& b = points_[i];
            const float span = b.x - a.x;
            if (span == 0.0f) return b.y;
            const float t = (input - a.x) / span;
            return a.y + (b.y - a.y) * t;
        }
    }
    return points_.back().y;
}

const std::vector<CurvePoint>& ResponseCurve::points() const noexcept { return points_; }
std::size_t ResponseCurve::size() const noexcept { return points_.size(); }

} // namespace mspa
