#pragma once

#include <string>
#include <vector>

#include "mspa/base/ids.hpp"

namespace mspa {

struct ActionOption {
    ActionTypeId type{};
    ActionInstanceId instance{};
    ActionContinuityKey continuity{};
    AgentId actor{};
    std::vector<AgentId> targets{};
    float cost{0.0f};
    float risk{0.0f};
    float progress{0.0f};
    std::vector<std::string> context_tags{};
};

struct ConsiderationScore {
    std::string name{};
    float raw{0.0f};
    float weighted{0.0f};
};

struct CandidateScore {
    ActionInstanceId action{};
    float utility{0.0f};
    std::vector<ConsiderationScore> considerations{};
};

struct Decision {
    bool has_selection{false};
    ActionInstanceId selected_action{};
    AgentId selected_target{};
    float confidence{0.0f};
    std::string intention{};
    std::string expressed_emotion{};
    std::vector<std::string> outgoing_statements{};
    std::vector<CandidateScore> trace{};
};

class UtilityComposer {
public:
    [[nodiscard]] CandidateScore score(const ActionOption& option) const;
};

} // namespace mspa
