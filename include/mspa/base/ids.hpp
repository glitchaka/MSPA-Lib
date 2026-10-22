#pragma once

#include <cstdint>
#include <tuple>
#include <type_traits>

namespace mspa {

template <typename Tag, typename Rep = std::uint64_t>
class StrongId {
public:
    using rep_type = Rep;

    constexpr StrongId() noexcept = default;
    explicit constexpr StrongId(Rep value) noexcept : value_(value) {}

    [[nodiscard]] constexpr Rep value() const noexcept { return value_; }
    [[nodiscard]] constexpr bool valid() const noexcept { return value_ != Rep{}; }

    friend constexpr bool operator==(StrongId lhs, StrongId rhs) noexcept {
        return lhs.value_ == rhs.value_;
    }

    friend constexpr bool operator!=(StrongId lhs, StrongId rhs) noexcept {
        return !(lhs == rhs);
    }

    friend constexpr bool operator<(StrongId lhs, StrongId rhs) noexcept {
        return lhs.value_ < rhs.value_;
    }

private:
    Rep value_{};
};

struct RuntimeIdTag;
struct WorldIdTag;
struct NamespaceIdTag;
struct ConceptIdTag;
struct PredicateIdTag;
struct StimulusIdTag;
struct GoalIdTag;
struct BeliefIdTag;
struct MemoryIdTag;
struct ActionTypeIdTag;
struct ActionInstanceIdTag;
struct ActionContinuityKeyTag;
struct PackIdTag;
struct TimeDomainIdTag;

using RuntimeId = StrongId<RuntimeIdTag>;
using WorldId = StrongId<WorldIdTag>;
using NamespaceId = StrongId<NamespaceIdTag>;
using ConceptId = StrongId<ConceptIdTag>;
using PredicateId = StrongId<PredicateIdTag>;
using StimulusId = StrongId<StimulusIdTag>;
using GoalId = StrongId<GoalIdTag>;
using BeliefId = StrongId<BeliefIdTag>;
using MemoryId = StrongId<MemoryIdTag>;
using ActionTypeId = StrongId<ActionTypeIdTag>;
using ActionInstanceId = StrongId<ActionInstanceIdTag>;
using ActionContinuityKey = StrongId<ActionContinuityKeyTag>;
using PackId = StrongId<PackIdTag>;
using TimeDomainId = StrongId<TimeDomainIdTag>;

struct AgentId {
    WorldId world{};
    std::uint64_t local_id{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return world.valid() && local_id != 0;
    }

    friend constexpr bool operator==(const AgentId& lhs, const AgentId& rhs) noexcept {
        return lhs.world == rhs.world && lhs.local_id == rhs.local_id;
    }

    friend constexpr bool operator!=(const AgentId& lhs, const AgentId& rhs) noexcept {
        return !(lhs == rhs);
    }

    friend constexpr bool operator<(const AgentId& lhs, const AgentId& rhs) noexcept {
        return std::tie(lhs.world, lhs.local_id) < std::tie(rhs.world, rhs.local_id);
    }
};

static_assert(!std::is_convertible_v<WorldId, RuntimeId>, "Strong IDs must not be implicitly interchangeable");

} // namespace mspa
