#pragma once

#include <cstddef>
#include <map>

#include "mspa/base/result.hpp"
#include "mspa/model/agent_state.hpp"

namespace mspa {

class AgentStore {
public:
    Result<void> insert(AgentPsychState state);
    Result<void> erase(AgentId id);

    [[nodiscard]] bool contains(AgentId id) const noexcept;
    [[nodiscard]] AgentPsychState* find(AgentId id) noexcept;
    [[nodiscard]] const AgentPsychState* find(AgentId id) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::map<AgentId, AgentPsychState> agents_{};
};

} // namespace mspa
