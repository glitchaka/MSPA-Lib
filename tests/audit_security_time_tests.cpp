#include "test.hpp"
#include "mspa/mspa.hpp"
#include <cstdint>
#include <iostream>
#include <vector>
using namespace mspa;

namespace {
RegistrySnapshot time_registry(){
    RegistryBuilder b; PredicateSchema p{};p.id=PredicateId(1);p.name_space=NamespaceId(1);p.default_relevance=0.2f;
    require(b.add_predicate(p),"register time predicate");return b.publish();
}
void set_u32(std::vector<std::uint8_t>& b,std::size_t off,std::uint32_t v){require(off+4<=b.size(),"mutation offset in range");for(int i=0;i<4;++i)b[off+static_cast<std::size_t>(i)]=static_cast<std::uint8_t>(v>>(8*i));}
}

int main(){
    AgentPsychState s{};s.id={WorldId(1),7};s.last_processed_time=PsychTime(90);s.rng_state.state=42;
    ExportedAgent exported{s,{TimeDomainId(1),{10,1}},PsychTime(100)};
    auto encoded=serialize_agent(exported);require(encoded,"serialize hostile-input seed");const auto good=*encoded.value_if();

    // Truncation at several boundaries must always be a clean error.
    const std::size_t cuts[]={0,1,4,8,11,12,20,good.size()/2,good.size()-1};
    for(auto n:cuts){std::vector<std::uint8_t> x(good.begin(),good.begin()+static_cast<std::ptrdiff_t>(n));auto d=deserialize_agent(x);require(!d,"truncated bytes must be rejected");}

    // Unknown serialization version.
    auto unknown=good;set_u32(unknown,4,0x7fffffffu);auto u=deserialize_agent(unknown);require(!u&&u.error_if()->code==ErrorCode::VersionMismatch,"unknown serialization version must be VersionMismatch");

    // Absurd collection length: values count begins after the fixed 84-byte header/personality block.
    auto huge=good;require(huge.size()>88,"serialized seed large enough");set_u32(huge,84,0xffffffffu);auto h=deserialize_agent(huge);require(!h,"huge serialized collection length must be rejected");
    auto limited=deserialize_agent(good,SerializationOptions{16});require(!limited&&limited.error_if()->code==ErrorCode::QuotaExceeded,"byte quota must reject oversized payload cleanly");
    auto too_small=serialize_agent(exported,SerializationOptions{16});require(!too_small&&too_small.error_if()->code==ErrorCode::QuotaExceeded,"serializer byte quota must be enforced");

    // Fuzz-like deterministic hostile corpus: no crash; arbitrary input returns either a clean Result error or a valid parse.
    DeterministicRng rng(0x5eedULL);std::size_t rejected=0,accepted=0;
    for(int case_id=0;case_id<500;++case_id){const std::size_t n=static_cast<std::size_t>(rng.next_u32()%2048u);std::vector<std::uint8_t> x(n);for(auto&v:x)v=static_cast<std::uint8_t>(rng.next_u32());auto d=deserialize_agent(x);if(d)++accepted;else ++rejected;}
    require(rejected>0,"hostile corpus must exercise rejection paths");

    // Domain packs cannot bypass declaration caps or mutate RuntimeLimits.
    RuntimeConfig cfg{};cfg.runtime_id=RuntimeId(1);cfg.world_id=WorldId(1);cfg.time_domain={TimeDomainId(1),{1,1}};cfg.limits.max_beliefs_per_agent=5;
    auto rr=Runtime::create(cfg);require(rr,"security runtime");auto runtime=std::move(*rr.value_if());
    DomainPack pack{};pack.metadata.id=PackId(1);pack.metadata.name_space=NamespaceId(2);pack.metadata.name="oversized";
    pack.predicates.reserve(4097);for(std::uint64_t i=1;i<=4097;++i){PredicateSchema p{};p.id=PredicateId(i);p.name_space=NamespaceId(2);pack.predicates.push_back(p);}
    auto staged=runtime->stage_domain_pack(pack);require(!staged&&staged.error_if()->code==ErrorCode::QuotaExceeded,"oversized pack declaration must be rejected");
    require(runtime->config().limits.max_beliefs_per_agent==5,"failed pack must not raise runtime limits");

    // process_time is monotonic, event_time may refer to the past.
    auto reg=time_registry();RuntimeConfig tc{};tc.runtime_id=RuntimeId(2);tc.world_id=WorldId(1);tc.time_domain={TimeDomainId(1),{60,1}};auto trr=Runtime::create(tc,reg);require(trr,"time runtime");auto tr=std::move(*trr.value_if());AgentId a{WorldId(1),1};require(tr->create_agent(a,AgentInit{},PsychTime(10)),"time agent");
    ObserveInput late{};late.agent=a;late.proposition.predicate=PredicateId(1);late.proposition.name_space=NamespaceId(1);late.time.process_time=PsychTime(20);late.time.has_event_time=true;late.time.event_time=PsychTime(5);require(tr->observe(late),"late event_time must be accepted");
    ObserveInput regression=late;regression.time.process_time=PsychTime(19);auto back=tr->observe(regression);require(!back&&back.error_if()->code==ErrorCode::TimeRegression,"backward process_time must return TimeRegression");

    // RebaseRelative preserves real-time intervals across different tick scales.
    AgentPsychState ts{};ts.id={WorldId(1),9};ts.last_processed_time=PsychTime(90);BeliefState b{};b.id=BeliefId(1);b.proposition.predicate=PredicateId(1);b.proposition.name_space=NamespaceId(1);b.certainty=0.8f;b.updated_at=PsychTime(70);ts.beliefs.push_back(b);
    ExportedAgent te{ts,{TimeDomainId(10),{10,1}},PsychTime(100)};ImportOptions io{};io.destination_id={WorldId(2),9};io.import_time=PsychTime(1000);io.destination_seed_root=123;io.time_policy=ImportTimePolicy::RebaseRelative;
    auto imp=import_agent_state(te,{TimeDomainId(20),{20,1}},io);require(imp,"RebaseRelative import");const auto& dst=imp.value_if()->state;
    require(dst.last_processed_time==PsychTime(980),"10 origin ticks age -> 20 destination ticks");require(dst.beliefs[0].updated_at==PsychTime(940),"30 origin ticks age -> 60 destination ticks");
    require(dst.last_processed_time.ticks()-dst.beliefs[0].updated_at.ticks()==40,"relative two-second interval preserved as 40 destination ticks");

    std::cout<<"AUDIT hostile/time PASS fuzz_rejected="<<rejected<<" fuzz_accepted="<<accepted<<" bytes="<<good.size()<<"\n";
}
