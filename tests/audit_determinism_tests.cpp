#include "test.hpp"
#include "mspa/mspa.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
using namespace mspa;

namespace {
RegistrySnapshot audit_registry(){
    RegistryBuilder b;
    PredicateSchema p{};
    p.id=PredicateId(1); p.name_space=NamespaceId(1);
    p.default_relevance=0.55f; p.default_consequence=0.25f;
    require(b.add_predicate(p),"register audit predicate");
    return b.publish();
}
std::unique_ptr<Runtime> make_runtime(const RegistrySnapshot& reg,std::uint64_t runtime_id=1){
    RuntimeConfig c{}; c.runtime_id=RuntimeId(runtime_id); c.world_id=WorldId(1);
    c.time_domain={TimeDomainId(1),{60,1}}; c.seed_root=0x123456789abcdef0ULL;
    c.limits.max_memories_per_agent=64;
    auto rr=Runtime::create(c,reg); require(rr,"create audit runtime");
    return std::move(*rr.value_if());
}
ObserveInput obs(AgentId a,std::uint64_t t,bool positive=true){
    ObserveInput o{}; o.agent=a; o.proposition.predicate=PredicateId(1);
    o.proposition.name_space=NamespaceId(1); o.proposition.positive=positive;
    o.time.process_time=PsychTime(t); o.perceived_certainty=0.82f; o.source_trust=0.91f;
    return o;
}
std::vector<ActionOption> actions(AgentId a){
    ActionOption x{}; x.type=ActionTypeId(1); x.instance=ActionInstanceId(1);
    x.continuity=ActionContinuityKey(1); x.actor=a; x.cost=0.15f; x.risk=0.10f; x.progress=0.60f;
    ActionOption y=x; y.instance=ActionInstanceId(2); y.continuity=ActionContinuityKey(2);
    y.cost=0.18f; y.risk=0.05f; y.progress=0.45f;
    return {x,y};
}
std::vector<std::uint8_t> run_full(Runtime& r,AgentId a){
    require(r.create_agent(a,AgentInit{},PsychTime(0)),"create scenario agent");
    require(r.observe(obs(a,10)),"scenario observe");
    auto d=r.decide(a,actions(a),PsychTime(11)); require(d&&d.value_if()->has_selection,"scenario decide");
    ReportInput rep{}; rep.agent=a; rep.action_type=ActionTypeId(1); rep.success=true;
    rep.context_tags={"audit","clear"}; rep.process_time=PsychTime(12);
    require(r.report(rep),"scenario report");
    require(r.advance_to(a,PsychTime(100)),"scenario advance");
    auto bytes=r.export_agent_bytes(a,PsychTime(100)); require(bytes,"scenario export bytes");
    return *bytes.value_if();
}
std::vector<std::uint8_t> run_batch(bool shuffled){
    auto reg=audit_registry(); auto r=make_runtime(reg,3); AgentId a{WorldId(1),77};
    require(r->create_agent(a,AgentInit{},PsychTime(0)),"batch create");
    ObserveBatchItem one{}; one.input_sequence=1; one.input=obs(a,10,true);
    ObserveBatchItem two{}; two.input_sequence=2; two.input=obs(a,20,false);
    std::vector<ObserveBatchItem> items=shuffled?std::vector<ObserveBatchItem>{two,one}:std::vector<ObserveBatchItem>{one,two};
    require(r->observe_batch(items),"observe batch");
    DecideBatchItem di{}; di.input_sequence=3; di.agent=a; di.actions=actions(a); di.process_time=PsychTime(21);
    require(r->decide_batch({di}),"decide batch");
    ReportBatchItem ri{}; ri.input_sequence=4; ri.input.agent=a; ri.input.action_type=ActionTypeId(1);
    ri.input.success=true; ri.input.context_tags={"audit"}; ri.input.process_time=PsychTime(22);
    require(r->report_batch({ri}),"report batch");
    AdvanceBatchItem ai{}; ai.input_sequence=5; ai.agent=a; ai.target_time=PsychTime(100);
    require(r->advance_batch({ai}),"advance batch");
    auto bytes=r->export_agent_bytes(a,PsychTime(100)); require(bytes,"batch export");
    return *bytes.value_if();
}
std::vector<std::uint8_t> run_individual(){
    auto reg=audit_registry(); auto r=make_runtime(reg,3); AgentId a{WorldId(1),77};
    require(r->create_agent(a,AgentInit{},PsychTime(0)),"individual create");
    require(r->observe(obs(a,10,true)),"individual observe 1");
    require(r->observe(obs(a,20,false)),"individual observe 2");
    require(r->decide(a,actions(a),PsychTime(21)),"individual decide");
    ReportInput ri{}; ri.agent=a; ri.action_type=ActionTypeId(1); ri.success=true;
    ri.context_tags={"audit"}; ri.process_time=PsychTime(22); require(r->report(ri),"individual report");
    require(r->advance_to(a,PsychTime(100)),"individual advance");
    auto bytes=r->export_agent_bytes(a,PsychTime(100)); require(bytes,"individual export");
    return *bytes.value_if();
}
}

int main(){
    auto reg=audit_registry();
    auto r1=make_runtime(reg,1); auto r2=make_runtime(reg,1); AgentId a{WorldId(1),1};
    const auto bytes1=run_full(*r1,a); const auto bytes2=run_full(*r2,a);
    require(bytes1==bytes2,"same scenario twice must produce identical exported bytes");

    auto solo=make_runtime(reg,2); auto crowded=make_runtime(reg,2);
    const auto solo_bytes=run_full(*solo,a);
    AgentId other{WorldId(1),999}; (void)run_full(*crowded,other);
    const auto crowded_bytes=run_full(*crowded,a);
    require(solo_bytes==crowded_bytes,"other agents must not perturb an agent deterministic stream/state");

    const auto canonical=run_batch(false); const auto shuffled=run_batch(true);
    require(canonical==shuffled,"shuffled batch must canonicalize to identical final state");
    const auto individual=run_individual();
    require(canonical==individual,"batch and individual calls must produce identical final state");

    std::cout<<"AUDIT determinism PASS bytes="<<bytes1.size()<<"\n";
}
