#include "test.hpp"
#include "mspa/c_api.h"
#include "mspa/mspa.hpp"
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

using namespace mspa;

namespace {

mspa_runtime* make_c_runtime(std::uint64_t runtime_id,std::uint64_t world,std::uint64_t domain_id,
                             std::uint64_t ticks_num,std::uint64_t ticks_den,std::uint64_t seed,
                             bool diagnostics=false){
    mspa_runtime_config cfg{};cfg.runtime_id=runtime_id;cfg.world_id=world;cfg.time_domain_id=domain_id;cfg.ticks_per_second_num=ticks_num;cfg.ticks_per_second_den=ticks_den;cfg.seed_root=seed;cfg.diagnostics_enabled=diagnostics?1:0;
    mspa_runtime* runtime=nullptr;require(mspa_runtime_create(&cfg,&runtime)==MSPA_OK&&runtime,"create C ABI runtime");return runtime;
}
std::unique_ptr<Runtime> make_cpp_runtime(std::uint64_t runtime_id,std::uint64_t world,std::uint64_t domain_id,
                                          std::uint64_t ticks_num,std::uint64_t ticks_den,std::uint64_t seed,
                                          bool diagnostics=false){
    RuntimeConfig cfg{};cfg.runtime_id=RuntimeId(runtime_id);cfg.world_id=WorldId(world);cfg.time_domain={TimeDomainId(domain_id),{ticks_num,ticks_den}};cfg.seed_root=seed;cfg.diagnostics_enabled=diagnostics;
    auto runtime=Runtime::create(cfg);require(runtime,"create C++ equivalence runtime");return std::move(*runtime.value_if());
}
std::vector<std::uint8_t> export_c(mspa_runtime* runtime,mspa_agent_id agent,std::uint64_t at){mspa_buffer b{};require(mspa_agent_export(runtime,agent,at,&b)==MSPA_OK,"C ABI export agent");std::vector<std::uint8_t> bytes;if(b.size)bytes.assign(b.data,b.data+b.size);mspa_buffer_free(&b);return bytes;}

mspa_observe_input c_observation(mspa_agent_id agent,std::uint64_t time){mspa_observe_input i{};i.agent=agent;i.predicate_id=1;i.namespace_id=1;i.positive=1;i.process_time=time;i.perceived_certainty=0.8f;i.source_trust=0.9f;return i;}
ObserveInput cpp_observation(AgentId agent,std::uint64_t time){ObserveInput i{};i.agent=agent;i.proposition.predicate=PredicateId(1);i.proposition.name_space=NamespaceId(1);i.proposition.positive=true;i.time.process_time=PsychTime(time);i.perceived_certainty=0.8f;i.source_trust=0.9f;return i;}

struct PairBytes{std::vector<std::uint8_t> low,high;};
PairBytes run_c_batch(bool reverse,bool individual){
    mspa_runtime* runtime=make_c_runtime(reverse?510:individual?511:512,7,1,60,1,0x1234,false);
    const mspa_agent_id low{7,1},high{7,2};require(mspa_agent_create(runtime,low,0)==MSPA_OK,"C batch create low");require(mspa_agent_create(runtime,high,0)==MSPA_OK,"C batch create high");
    if(individual){mspa_observe_result out{};auto a=c_observation(low,10);auto b=c_observation(high,10);require(mspa_observe(runtime,&a,&out)==MSPA_OK,"C individual low observe");require(mspa_observe(runtime,&b,&out)==MSPA_OK,"C individual high observe");}
    else{
        mspa_observe_batch_item items[2]{};items[0].input_sequence=1;items[1].input_sequence=1;
        if(reverse){items[0].input=c_observation(high,10);items[1].input=c_observation(low,10);}else{items[0].input=c_observation(low,10);items[1].input=c_observation(high,10);}
        mspa_observe_batch_output outputs[2]{};size_t count=0;require(mspa_observe_batch(runtime,items,2,outputs,2,&count)==MSPA_OK&&count==2,"C observe batch");
        require(outputs[0].agent.world==7&&outputs[0].agent.local_id==1&&outputs[1].agent.local_id==2,"C batch output is canonical");
    }
    PairBytes result{export_c(runtime,low,10),export_c(runtime,high,10)};mspa_runtime_destroy(runtime);return result;
}
PairBytes run_cpp_batch(){
    auto runtime=make_cpp_runtime(512,7,1,60,1,0x1234,false);const AgentId low{WorldId(7),1},high{WorldId(7),2};require(runtime->create_agent(low,AgentInit{},PsychTime(0)),"C++ batch low");require(runtime->create_agent(high,AgentInit{},PsychTime(0)),"C++ batch high");
    ObserveBatchItem a{};a.input_sequence=1;a.input=cpp_observation(high,10);ObserveBatchItem b{};b.input_sequence=1;b.input=cpp_observation(low,10);auto q=runtime->observe_batch({a,b});require(q&&q.value_if()->size()==2,"C++ batch equivalent");require((*q.value_if())[0].agent==low&&(*q.value_if())[1].agent==high,"C++ canonical order");auto lowb=runtime->export_agent_bytes(low,PsychTime(10));auto highb=runtime->export_agent_bytes(high,PsychTime(10));require(lowb&&highb,"C++ batch export");return {*lowb.value_if(),*highb.value_if()};
}

struct CScenarioResult{std::vector<std::uint8_t> bytes{};std::uint64_t selected{0};float confidence{0};};
CScenarioResult run_c_learning_scenario(bool add_other=false){
    mspa_runtime* runtime=make_c_runtime(500,7,1,60,1,555,false);const mspa_agent_id agent{7,10};
    if(add_other){const mspa_agent_id other{7,999};require(mspa_agent_create(runtime,other,0)==MSPA_OK,"C unrelated create");require(mspa_report(runtime,other,99,1,1)==MSPA_OK,"C unrelated report");mspa_action_option o{};o.type_id=99;o.instance_id=99;o.context_stability=1.0f;mspa_decision_result d{};require(mspa_decide(runtime,other,&o,1,2,0,&d)==MSPA_OK,"C unrelated decide");}
    require(mspa_agent_create(runtime,agent,0)==MSPA_OK,"C create learning agent");const char* rain[]={"rain"};const char* sun[]={"sun"};
    for(std::uint64_t i=1;i<=8;++i){require(mspa_report_ex(runtime,agent,10,1,rain,1,nullptr,0,i)==MSPA_OK,"C success");require(mspa_report_ex(runtime,agent,11,0,rain,1,nullptr,0,i)==MSPA_OK,"C failure");}
    for(std::uint64_t i=10;i<=17;++i){require(mspa_report_ex(runtime,agent,12,1,rain,1,nullptr,0,i)==MSPA_OK,"C rain specialization");require(mspa_report_ex(runtime,agent,12,0,sun,1,nullptr,0,i)==MSPA_OK,"C sun specialization");}
    mspa_action_option opts[2]{};opts[0].type_id=11;opts[0].instance_id=11;opts[0].context_stability=1;opts[0].context_tags=rain;opts[0].context_tag_count=1;opts[1]=opts[0];opts[1].type_id=10;opts[1].instance_id=10;mspa_decision_result d{};require(mspa_decide(runtime,agent,opts,2,18,0,&d)==MSPA_OK&&d.has_selection,"C learning DECIDE");
    CScenarioResult result{export_c(runtime,agent,18),d.selected_action,d.confidence};mspa_runtime_destroy(runtime);return result;
}
CScenarioResult run_cpp_learning_scenario(){
    auto runtime=make_cpp_runtime(500,7,1,60,1,555,false);const AgentId agent{WorldId(7),10};require(runtime->create_agent(agent,AgentInit{},PsychTime(0)),"C++ learning agent");
    for(std::uint64_t i=1;i<=8;++i){ReportInput good{};good.agent=agent;good.action_type=ActionTypeId(10);good.success=true;good.context_tags={"rain"};good.process_time=PsychTime(i);require(runtime->report(good),"C++ success");auto bad=good;bad.action_type=ActionTypeId(11);bad.success=false;require(runtime->report(bad),"C++ failure");}
    for(std::uint64_t i=10;i<=17;++i){ReportInput rain{};rain.agent=agent;rain.action_type=ActionTypeId(12);rain.success=true;rain.context_tags={"rain"};rain.process_time=PsychTime(i);require(runtime->report(rain),"C++ rain specialization");auto sun=rain;sun.success=false;sun.context_tags={"sun"};require(runtime->report(sun),"C++ sun specialization");}
    ActionOption bad{};bad.type=ActionTypeId(11);bad.instance=ActionInstanceId(11);bad.actor=agent;bad.context_tags={"rain"};ActionOption good=bad;good.type=ActionTypeId(10);good.instance=ActionInstanceId(10);auto d=runtime->decide(agent,{bad,good},PsychTime(18));require(d&&d.value_if()->has_selection,"C++ learning decide");auto bytes=runtime->export_agent_bytes(agent,PsychTime(18));require(bytes,"C++ learning export");return {*bytes.value_if(),d.value_if()->selected_action.value(),d.value_if()->confidence};
}

ExportedAgent complex_cross_scale_source(){
    ExportedAgent e{};e.origin_time_domain={TimeDomainId(10),{10,1}};e.exported_at=PsychTime(100);e.state.id={WorldId(7),5};e.state.last_processed_time=PsychTime(90);e.state.rng_state.state=0x1111222233334444ULL;e.state.mood={-0.2f,0.4f,PsychTime(80)};
    BeliefState b{};b.id=BeliefId(1);b.proposition.predicate=PredicateId(1);b.proposition.name_space=NamespaceId(1);b.proposition.has_time=true;b.proposition.time=PsychTime(60);b.certainty=0.8f;b.updated_at=PsychTime(70);e.state.beliefs.push_back(b);
    e.state.emotions.push_back({BasicEmotion::Fear,0.4f,PsychTime(50),PsychTime(75)});
    GoalState g{};g.id=GoalId(1);g.concept_id=ConceptId(1);g.priority=0.6f;g.commitment=0.7f;g.created_at=PsychTime(40);g.updated_at=PsychTime(85);e.state.goals.push_back(g);
    MemoryState m{};m.id=MemoryId(1);m.proposition.predicate=PredicateId(1);m.proposition.name_space=NamespaceId(1);m.proposition.has_time=true;m.proposition.time=PsychTime(55);m.importance=0.7f;m.certainty=0.8f;m.event_time=PsychTime(30);m.last_reactivated_at=PsychTime(65);e.state.memories.push_back(m);
    LearnedExpectation le{};le.action=ActionTypeId(7);le.context_tags={"rain"};le.outcome_tag="success";le.attempts=2;le.successes=1;le.confidence=0.6f;le.last_activation=PsychTime(45);e.state.learned_expectations.push_back(le);return e;
}

} // namespace

int main(){
    // A12/A1: repeated scenario, unrelated-agent isolation, shuffled/canonical batch and batch==individual.
    const auto c_a=run_c_learning_scenario(false),c_b=run_c_learning_scenario(false),c_other=run_c_learning_scenario(true),cpp_learning=run_cpp_learning_scenario();
    require(c_a.bytes==c_b.bytes,"A12 C repeated scenario deterministic");require(c_a.bytes==c_other.bytes,"A12 C unrelated agent does not perturb target");require(c_a.bytes==cpp_learning.bytes,"A12 C learning state equals C++");require(c_a.selected==cpp_learning.selected&&c_a.selected==10,"A12 C/C++ learned selection equal");require(c_a.confidence==cpp_learning.confidence,"A12 C/C++ confidence equal");
    auto decoded_learning=deserialize_agent(c_a.bytes);require(decoded_learning,"decode C learning state");const auto& learned=decoded_learning.value_if()->state.learned_expectations;auto find=[&](const char* tag){for(const auto&e:learned)if(e.action==ActionTypeId(12)&&e.context_tags.size()==1&&e.context_tags.front()==tag&&e.outcome_tag=="success")return e.confidence;return -1.0f;};require(find("rain")>find("sun")+0.5f,"A12/A4 C context specialization");
    const auto c_reverse=run_c_batch(true,false),c_canonical=run_c_batch(false,false),c_individual=run_c_batch(false,true),cpp_batch=run_cpp_batch();require(c_reverse.low==c_canonical.low&&c_reverse.high==c_canonical.high,"A12/A1 C shuffled batch equals canonical");require(c_reverse.low==c_individual.low&&c_reverse.high==c_individual.high,"A12/A1 C batch equals individual");require(c_reverse.low==cpp_batch.low&&c_reverse.high==cpp_batch.high,"A12/A1 C batch bytes equal C++ batch");

    // A12/A9: complex RebaseRelative and RekeyForDestination through C equal C++ for two destinations.
    auto source=complex_cross_scale_source();auto source_bytes=serialize_agent(source);require(source_bytes,"serialize complex C import source");auto cpp_dest=make_cpp_runtime(502,8,20,20,1,999,false);ImportOptions o{};o.import_time=PsychTime(1000);o.time_policy=ImportTimePolicy::RebaseRelative;o.rng_policy=RngImportPolicy::RekeyForDestination;o.destination_seed_root=999;o.destination_id={WorldId(8),6};auto cpp6=cpp_dest->import_agent_bytes(*source_bytes.value_if(),o);require(cpp6,"C++ import 6");o.destination_id={WorldId(8),7};auto cpp7=cpp_dest->import_agent_bytes(*source_bytes.value_if(),o);require(cpp7,"C++ import 7");auto cpp6b=cpp_dest->export_agent_bytes({WorldId(8),6},PsychTime(1000));auto cpp7b=cpp_dest->export_agent_bytes({WorldId(8),7},PsychTime(1000));require(cpp6b&&cpp7b,"C++ import exports");
    mspa_runtime* c_dest=make_c_runtime(502,8,20,20,1,999,false);mspa_agent_id id6{8,6},id7{8,7};require(mspa_agent_import(c_dest,source_bytes.value_if()->data(),source_bytes.value_if()->size(),id6,1000)==MSPA_OK,"C import 6");require(mspa_agent_import(c_dest,source_bytes.value_if()->data(),source_bytes.value_if()->size(),id7,1000)==MSPA_OK,"C import 7");auto c6=export_c(c_dest,id6,1000),c7=export_c(c_dest,id7,1000);require(c6==*cpp6b.value_if()&&c7==*cpp7b.value_if(),"A12/A9 C imports byte-equal C++");auto d6=deserialize_agent(c6),d7=deserialize_agent(c7);require(d6&&d7,"decode C imported states");require(d6.value_if()->state.last_processed_time==PsychTime(980)&&d6.value_if()->state.mood.updated_at==PsychTime(960)&&d6.value_if()->state.memories.front().event_time==PsychTime(860),"A12/A9 C rebase preserves relative timestamps");require(d6.value_if()->state.rng_state.state!=source.state.rng_state.state&&d6.value_if()->state.rng_state.state!=d7.value_if()->state.rng_state.state,"A12/A9 C rekeys distinct destinations independently");mspa_runtime_destroy(c_dest);

    // A12/A9 time semantics and no exception escape.
    mspa_runtime* c_time=make_c_runtime(503,9,1,60,1,123,false);mspa_agent_id ta{9,1};require(mspa_agent_create(c_time,ta,10)==MSPA_OK,"C time agent");auto obs=c_observation(ta,20);obs.has_event_time=1;obs.event_time=5;mspa_observe_result orr{};require(mspa_observe(c_time,&obs,&orr)==MSPA_OK,"A12 historical event time");obs.process_time=19;require(mspa_observe(c_time,&obs,&orr)==MSPA_TIME_REGRESSION,"A12 backward process time");bool escaped=false;try{require(mspa_agent_create(nullptr,ta,0)==MSPA_INVALID_ARGUMENT,"C null runtime status");require(mspa_agent_create(c_time,ta,20)==MSPA_DUPLICATE_AGENT,"C duplicate status");const std::uint8_t malformed[]={0,1,2,3};require(mspa_agent_import(c_time,malformed,sizeof(malformed),{9,2},30)!=MSPA_OK,"C malformed import status");}catch(...){escaped=true;}require(!escaped,"A12 no C exception escapes");mspa_runtime_destroy(c_time);

    std::cout<<"AUDIT_PHASE2_C_ABI PASS learned_action="<<c_a.selected<<" bytes="<<c_a.bytes.size()<<" batch_low="<<c_reverse.low.size()<<" rekey="<<d6.value_if()->state.rng_state.state<<"/"<<d7.value_if()->state.rng_state.state<<"\n";
}
