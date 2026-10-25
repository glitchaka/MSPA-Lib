#include "test.hpp"
#include "mspa/mspa.hpp"
#include <cmath>
using namespace mspa;
int main(){
    ProfileRegistry registry;require(install_builtin_profiles(registry),"install built-in profiles");require(registry.size()==3,"exactly three built-in profiles");
    require(registry.contains(builtin_profile_ids::balanced),"balanced exists");require(registry.contains(builtin_profile_ids::cautious_persistent),"cautious persistent exists");require(registry.contains(builtin_profile_ids::curious_bold),"curious bold exists");
    for(std::uint64_t i=100;i<110;++i){PsychologicalProfile p{};p.id=ProfileId(i);p.name="custom_"+std::to_string(i);p.personality.sociability=static_cast<float>(i-100)/10.0f;p.values.push_back({ConceptId(700+i),0.6f});p.goals.push_back({ConceptId(900+i),0.7f,0.8f});require(registry.add(std::move(p)),"registry accepts more than three profiles");}
    require(registry.size()==13,"profile registry is not capped at three");
    ProfileRecipe mix{};mix.components={{builtin_profile_ids::cautious_persistent,1.0f},{builtin_profile_ids::curious_bold,1.0f}};auto resolved=registry.resolve(mix,{WorldId(1),1});require(resolved,"blend profiles");require(resolved.value_if()->personality.curiosity>0.38f&&resolved.value_if()->personality.curiosity<0.90f,"weighted blend curiosity");require(resolved.value_if()->personality.boldness>0.22f&&resolved.value_if()->personality.boldness<0.82f,"weighted blend boldness");
    ProfileRecipe varied=mix;varied.variation_seed=77;varied.variation_strength=0.1f;auto a=registry.resolve(varied,{WorldId(1),42});auto b=registry.resolve(varied,{WorldId(1),42});auto c=registry.resolve(varied,{WorldId(1),43});require(a&&b&&c,"deterministic variation resolves");require(a.value_if()->personality.sociability==b.value_if()->personality.sociability,"same agent and seed reproduce profile variation");require(a.value_if()->personality.sociability!=c.value_if()->personality.sociability||a.value_if()->personality.curiosity!=c.value_if()->personality.curiosity,"different agents can diverge");
    RuntimeConfig cfg{};cfg.runtime_id=RuntimeId(1);cfg.world_id=WorldId(1);cfg.time_domain={TimeDomainId(1),{1,1}};cfg.seed_root=5;auto rr=Runtime::create(cfg);require(rr,"runtime");auto runtime=std::move(*rr.value_if());
    ProfileRecipe custom{};custom.components={{ProfileId(100),1.0f}};AgentId id{WorldId(1),55};require(create_agent_from_profiles(*runtime,id,registry,custom,PsychTime(10)),"create agent from arbitrary registered profile");const auto* state=runtime->agents().find(id);require(state!=nullptr,"profiled agent exists");require(!state->values.empty()&&!state->goals.empty(),"profile seeds values and goals");require(state->mood.updated_at==PsychTime(10),"profile mood anchored to creation time");
    auto init=materialize_profile_init(registry,mix,{WorldId(1),56},PsychTime(20));require(init,"materialize profile init independently");init.value_if()->personality.empathy=1.0f;require(runtime->create_agent({WorldId(1),56},*init.value_if(),PsychTime(20)),"materialized init can be customized before creation");
    std::cout<<"profiles ok\n";
}
