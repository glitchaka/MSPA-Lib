#include "mspa/diagnostics/trace.hpp"

namespace mspa {

void TraceSink::push(DiagnosticTrace trace) {
    traces_.push_back(std::move(trace));
}

const std::vector<DiagnosticTrace>& TraceSink::traces() const noexcept {
    return traces_;
}

void TraceSink::clear() noexcept {
    traces_.clear();
}

} // namespace mspa
