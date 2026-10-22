#pragma once

#include <string>
#include <vector>

#include "mspa/base/ids.hpp"
#include "mspa/base/time.hpp"

namespace mspa {

struct TraceItem {
    std::string stage{};
    std::string detail{};
    float weight{0.0f};
};

struct DiagnosticTrace {
    AgentId agent{};
    PsychTime process_time{};
    bool has_event_time{false};
    PsychTime event_time{};
    std::vector<TraceItem> items{};
};

class TraceSink {
public:
    void push(DiagnosticTrace trace);
    [[nodiscard]] const std::vector<DiagnosticTrace>& traces() const noexcept;
    void clear() noexcept;

private:
    std::vector<DiagnosticTrace> traces_{};
};

} // namespace mspa
