#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "mspa/base/ids.hpp"
#include "mspa/base/rng.hpp"
#include "mspa/base/time.hpp"

namespace mspa {

struct Personality {
    float sociability{0.5f};
    float stability{0.5f};
    float empathy{0.5f};
    float persistence{0.5f};
    float curiosity{0.5f};
    float boldness{0.5f};
};

struct ValueEntry {
    ConceptId concept{};
    float intensity{0.0f};
};

struct BeliefState {
    BeliefId id{};
    PredicateId predicate{};
    float certainty{0.0f};
    float emotional_load{0.0f};
    PsychTime updated_at{};
};

struct EmotionState {
    ConceptId kind{};
    float intensity{0.0f};
    PsychTime started_at{};
};

struct MoodState {
    float valence{0.0f};
    float activation{0.0f};
    PsychTime updated_at{};
};

struct GoalState {
    GoalId id{};
    ConceptId concept{};
    float priority{0.0f};
    bool active{true};
};

struct MemoryState {
    MemoryId id{};
    PredicateId predicate{};
    float importance{0.0f};
    PsychTime event_time{};
};

struct RelationshipState {
    AgentId other{};
    float affection{0.0f};
    float trust{0.0f};
    float respect{0.0f};
    float familiarity{0.0f};
};

struct LearnedExpectation {
    ActionTypeId action{};
    std::uint32_t attempts{0};
    std::uint32_t successes{0};
    float confidence{0.0f};
};

struct AgentPsychState {
    AgentId id{};
    Personality personality{};
    std::vector<ValueEntry> values{};
    std::vector<BeliefState> beliefs{};
    std::vector<EmotionState> emotions{};
    MoodState mood{};
    std::vector<GoalState> goals{};
    std::vector<MemoryState> memories{};
    std::vector<RelationshipState> relationships{};
    std::vector<LearnedExpectation> learned_expectations{};
    ActionContinuityKey current_intent{};
    RngState rng_state{};
    PsychTime last_processed_time{};
};

} // namespace mspa
