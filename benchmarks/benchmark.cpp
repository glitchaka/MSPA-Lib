#include "mspa/mspa.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>
using namespace mspa;
namespace {
RegistrySnapshot registry(){RegistryBuilder b;PredicateSchema p{};p.id=PredicateId(1);p.name_space=NamespaceId(1);p.default_relevance=0.4f;p.default_consequence=0.1f;if(!b.add_predicate(p))return {};return b.publish();}
std::unique_ptr<Runtime> runtime(std::size_t n,const RegistrySnapshot& reg,std::uint64_t rid){RuntimeConfig c{};c.runtime_id=RuntimeId(rid);c.world_id=WorldId(1);c.time_domain={TimeDomainId(1),{60,1}};c.seed_root=42;c.limits.max_agents=std::max<std::size_t>(n+8,100);c.limits.max_memories_per_agent=16;auto rr=Runtime::create(c,reg);if(!rr)return{};auto r=std::move(*rr.value_if());for(std::size_t i=1;i<=n;++i)if(!r->create_agent({WorldId(1),static_cast<std::uint64_t>(i)},AgentInit{},PsychTime(0)))return{};return r;}
void print_line(const char* op,std::size_t agents,std::uint64_t operations,double seconds){const double rate=seconds>0.0?static_cast<double>(operations)/seconds:0.0;std::cout<<"BENCH op="<<op<<" agents="<<agents<<" operations="<<operations<<" seconds="<<std::fixed<<std::setprecision(6)<<seconds<<" ops_per_sec="<<std::setprecision(2)<<rate<<"\n";}
bool observe_bench(std::size_t n,const RegistrySnapshot& reg){auto r=runtime(n,reg,100+n);if(!r)return false;const std::size_t rounds=std::max<std::size_t>(10,100000/n);Proposition p{};p.predicate=PredicateId(1);p.name_space=NamespaceId(1);const auto start=std::chrono::steady_clock::now();for(std::size_t round=0;round<rounds;++round){for(std::size_t i=1;i<=n;++i){ObserveInput o{};o.agent={WorldId(1),static_cast<std::uint64_t>(i)};o.proposition=p;o.time.process_time=PsychTime(static_cast<std::uint64_t>(round+1));o.perceived_certainty=0.8f;o.source_trust=0.8f;if(!r->observe(o))return false;}}const auto end=std::chrono::steady_clock::now();const auto operations=static_cast<std::uint64_t>(n*rounds);const double seconds=std::chrono::duration<double>(end-start).count();print_line("OBSERVE",n,operations,seconds);return true;}
bool decide_bench(std::size_t n,const RegistrySnapshot& reg){auto r=runtime(n,reg,200+n);if(!r)return false;const std::size_t rounds=std::max<std::size_t>(10,100000/n);const auto start=std::chrono::steady_clock::now();for(std::size_t round=0;round<rounds;++round){for(std::size_t i=1;i<=n;++i){AgentId a{WorldId(1),static_cast<std::uint64_t>(i)};ActionOption x{};x.type=ActionTypeId(1);x.instance=ActionInstanceId(1);x.continuity=ActionContinuityKey(1);x.actor=a;x.cost=0.1f;x.risk=0.2f;x.progress=0.6f;ActionOption y=x;y.type=ActionTypeId(2);y.instance=ActionInstanceId(2);y.continuity=ActionContinuityKey(2);y.cost=0.2f;y.risk=0.1f;y.progress=0.5f;if(!r->decide(a,{x,y},PsychTime(static_cast<std::uint64_t>(round+1))))return false;}}const auto end=std::chrono::steady_clock::now();const auto operations=static_cast<std::uint64_t>(n*rounds);const double seconds=std::chrono::duration<double>(end-start).count();print_line("DECIDE",n,operations,seconds);return true;}
}
int main(){const auto reg=registry();for(const std::size_t n:{100u,1000u,10000u}){if(!observe_bench(n,reg))return 1;if(!decide_bench(n,reg))return 2;}return 0;}
