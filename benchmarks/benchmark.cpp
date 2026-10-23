#include "mspa/mspa.hpp"
#include <chrono>
#include <iostream>
using namespace mspa;
int main(){RuntimeConfig c{};c.runtime_id=RuntimeId(1);c.world_id=WorldId(1);c.time_domain={TimeDomainId(1),{1,1}};auto rr=Runtime::create(c);if(!rr)return 1;auto r=std::move(*rr.value_if());constexpr int N=10000;for(int i=1;i<=N;++i)if(!r->create_agent({WorldId(1),static_cast<std::uint64_t>(i)},AgentInit{},PsychTime(0)))return 2;ActionOption o{};o.type=ActionTypeId(1);o.instance=ActionInstanceId(1);o.continuity=ActionContinuityKey(1);o.progress=0.5f;auto start=std::chrono::steady_clock::now();for(int i=1;i<=N;++i){AgentId a{WorldId(1),static_cast<std::uint64_t>(i)};o.actor=a;if(!r->decide(a,{o},PsychTime(1)))return 3;}auto end=std::chrono::steady_clock::now();auto us=std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();std::cout<<N<<" decisions in "<<us<<" us\n";}
