#include "test.hpp"
#include "mspa/mspa.hpp"
#include "mspa/psychology/learning.hpp"
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

using namespace mspa;

namespace {
std::unique_ptr<Runtime> make_runtime() {
    RuntimeConfig config{};
    config.runtime_id = RuntimeId(1);
    config.world_id = WorldId(1);
    config.time_domain = {TimeDomainId(1), {1, 1}};
    config.seed_root = 99;
    config.limits.max_agents = 16;
    auto created = Runtime::create(config);
    require(created, "create runtime");
    return std::move(*created.value_if());
}

Proposition fact(std::uint64_t predicate) {
    Proposition p{};
    p.predicate = PredicateId(predicate);
    p.name_space = NamespaceId(1);
    p.provenance = "parent-personal-history";
    return p;
}

AgentInit parent_init(float sociability, float persistence, AgentId other) {
    AgentInit init{};
    init.personality.sociability = sociability;
    init.personality.persistence = persistence;
    init.values.push_back({ConceptId(1), 0.8f});

    BeliefState belief{};
    belief.id = BeliefId(1);
    belief.proposition = fact(1);
    belief.certainty = 0.9f;
    init.beliefs.push_back(belief);

    MemoryState memory{};
    memory.id = MemoryId(1);
    memory.proposition = fact(2);
    memory.importance = 0.8f;
    memory.certainty = 1.0f;
    init.memories.push_back(memory);

    init.relationships.push_back({other, 0.5f, 0.4f, 0.3f, 0.6f});
    return init;
}

void report(Runtime& runtime, AgentId agent, ActionTypeId action, bool success,
            std::uint64_t time, const char* context) {
    ReportInput input{};
    input.agent = agent;
    input.action_type = action;
    input.success = success;
    input.process_time = PsychTime(time);
    input.context_tags = {context};
    require(runtime.report(input), "report training result");
}

std::vector<ActionOption> options(AgentId actor) {
    ActionOption inherited{};
    inherited.type = ActionTypeId(10);
    inherited.instance = ActionInstanceId(1);
    inherited.actor = actor;
    inherited.cost = 0.15f;
    inherited.context_tags = {"forest"};

    ActionOption neutral = inherited;
    neutral.type = ActionTypeId(11);
    neutral.instance = ActionInstanceId(2);
    neutral.cost = 0.0f;
    return {inherited, neutral};
}

float expected(const Runtime& runtime, AgentId id, ActionTypeId action) {
    const auto* state = runtime.agents().find(id);
    require(state != nullptr, "agent state exists");
    return LearningSystem{}.expected_success(*state, action, {"forest"});
}
} // namespace

int main() {
    auto runtime = make_runtime();
    const AgentId p1{WorldId(1), 1};
    const AgentId p2{WorldId(1), 2};
    require(runtime->create_agent(p1, parent_init(0.8f, 0.7f, p2), PsychTime(0)), "create parent one");
    require(runtime->create_agent(p2, parent_init(0.4f, 0.5f, p1), PsychTime(0)), "create parent two");

    for (std::uint64_t i = 1; i <= 12; ++i) {
        report(*runtime, p1, ActionTypeId(10), true, i, "forest");
        report(*runtime, p2, ActionTypeId(10), true, i, "forest");
    }
    for (std::uint64_t i = 13; i <= 24; ++i) {
        report(*runtime, p1, ActionTypeId(11), false, i, "forest");
        report(*runtime, p2, ActionTypeId(11), false, i, "forest");
    }

    const float parent_success = expected(*runtime, p1, ActionTypeId(10));
    require(parent_success > 0.8f, "parents must learn the successful action strongly");

    DescendantInit inherited{};
    inherited.parent_agents = {p1, p2};
    inherited.inherited_learning_policy = InheritedLearningPolicy::Mean;
    inherited.inherited_learning_evidence = 0.5f;
    const AgentId child{WorldId(1), 3};
    require(runtime->create_agent_descendant(child, inherited, PsychTime(100)), "create descendant with inherited learning");

    const auto* child_state = runtime->agents().find(child);
    require(child_state != nullptr, "descendant exists");
    require(std::fabs(child_state->personality.sociability - 0.6f) < 1.0e-6f,
            "descendant blends parent personality");
    require(child_state->beliefs.empty(), "descendant must not inherit autobiographical beliefs");
    require(child_state->memories.empty(), "descendant must not inherit autobiographical memories");
    require(child_state->relationships.empty(), "descendant must not inherit personal relationships");
    require(!child_state->learned_expectations.empty(), "descendant inherits action expectations");

    const float inherited_success = expected(*runtime, child, ActionTypeId(10));
    require(inherited_success > 0.5f, "inherited successful action starts above neutral");
    require(inherited_success < parent_success, "inherited learning is weaker than parent experience");
    require(expected(*runtime, child, ActionTypeId(11)) < 0.5f,
            "inherited failed action starts below neutral");

    DescendantInit none = inherited;
    none.inherited_learning_policy = InheritedLearningPolicy::None;
    const AgentId control{WorldId(1), 4};
    require(runtime->create_agent_descendant(control, none, PsychTime(100)), "create no-learning control descendant");
    require(std::fabs(expected(*runtime, control, ActionTypeId(10)) - 0.5f) < 1.0e-6f,
            "control descendant starts neutral without learning inheritance");

    auto control_decision = runtime->decide(control, options(control), PsychTime(100));
    require(control_decision && control_decision.value_if()->selected_action == ActionInstanceId(2),
            "without inheritance the lower-cost action wins");
    auto inherited_decision = runtime->decide(child, options(child), PsychTime(100));
    require(inherited_decision && inherited_decision.value_if()->selected_action == ActionInstanceId(1),
            "inherited experience changes descendant behavior before personal experience");

    for (std::uint64_t i = 101; i <= 124; ++i)
        report(*runtime, child, ActionTypeId(10), false, i, "forest");
    require(expected(*runtime, child, ActionTypeId(10)) < 0.5f,
            "descendant can overturn inherited learning through personal failures");
    auto revised_decision = runtime->decide(child, options(child), PsychTime(125));
    require(revised_decision && revised_decision.value_if()->selected_action == ActionInstanceId(2),
            "personal experience can reverse the inherited behavioral preference");

    DescendantInit invalid = inherited;
    invalid.inherited_learning_evidence = 1.5f;
    require(!runtime->create_agent_descendant({WorldId(1), 5}, invalid, PsychTime(100)),
            "invalid inheritance strength is rejected");

    std::cout << "descendant inheritance ok parent=" << parent_success
              << " inherited=" << inherited_success
              << " revised=" << expected(*runtime, child, ActionTypeId(10)) << '\n';
}
