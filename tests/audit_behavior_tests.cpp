#include "test.hpp"
#include "mspa/mspa.hpp"
#include "mspa/psychology/beliefs.hpp"
#include "mspa/psychology/emotions.hpp"
#include "mspa/psychology/learning.hpp"
#include "mspa/psychology/reappraisal.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
using namespace mspa;

namespace {
RegistrySnapshot behavior_registry(){
    RegistryBuilder b;
    PredicateSchema p{}; p.id=PredicateId(1); p.name_space=NamespaceId(1);
    p.default_relevance=0.75f; p.default_consequence=-0.55f; p.appraisal_hints={"threat"};
    require(b.add_predicate(p),"register behavior predicate"); return b.publish();
}
std::unique_ptr<Runtime> make_runtime(std::uint64_t rid,const RegistrySnapshot& reg=RegistrySnapshot{}){
    RuntimeConfig c{}; c.runtime_id=RuntimeId(rid); c.world_id=WorldId(1);
    c.time_domain={TimeDomainId(1),{60,1}}; c.seed_root=12345;
    auto rr=Runtime::create(c,reg); require(rr,"create behavior runtime"); return std::move(*rr.value_if());
}
PsychologicalProfile profile(std::uint64_t id,const std::string& name,const Personality& p){
    PsychologicalProfile x{}; x.id=ProfileId(id); x.name=name; x.personality=p; return x;
}
float fear_of(const AgentPsychState& a){return EmotionSystem{}.intensity(a,BasicEmotion::Fear);}
std::vector<ActionOption> near_tie(AgentId a,bool continuity,bool flip){
    ActionOption x{}; x.type=ActionTypeId(1); x.instance=ActionInstanceId(1); x.actor=a;
    x.continuity=continuity?ActionContinuityKey(1):ActionContinuityKey{}; x.cost=flip?0.11f:0.10f;
    x.progress=0.35f; x.interruption_cost=0.30f; x.context_stability=1.0f;
    ActionOption y=x; y.instance=ActionInstanceId(2); y.continuity=continuity?ActionContinuityKey(2):ActionContinuityKey{};
    y.cost=flip?0.10f:0.11f; return {x,y};
}
int switch_count(bool continuity){
    auto r=make_runtime(continuity?20:21); AgentId a{WorldId(1),1};
    AgentInit init{}; init.personality.persistence=0.85f; init.personality.stability=0.85f;
    require(r->create_agent(a,init,PsychTime(0)),"hysteresis agent");
    std::uint64_t previous=0; int switches=0;
    for(int i=0;i<100;++i){auto d=r->decide(a,near_tie(a,continuity,(i%2)==1),PsychTime(static_cast<std::uint64_t>(i+1)));
        require(d&&d.value_if()->has_selection,"hysteresis decide"); const auto now=d.value_if()->selected_action.value();
        if(previous&&now!=previous)++switches; previous=now;}
    return switches;
}
void check_trace(const Decision& d){
    require(d.has_selection,"trace decision has selection");
    float best=-1.0e30f; ActionInstanceId best_id{};
    for(const auto& candidate:d.trace){float sum=0.0f; for(const auto& c:candidate.considerations)sum+=c.weighted;
        require(std::fabs(sum-candidate.utility)<1.0e-5f,"trace contributions must sum to candidate utility");
        if(candidate.utility>best){best=candidate.utility;best_id=candidate.action;}}
    require(best_id==d.selected_action,"trace utilities and guards must reproduce selected action");
}
}

int main(){
    // 1) Same evidence, different profiles -> different beliefs.
    ProfileRegistry profiles;
    Personality rigid{}; rigid.persistence=0.95f; rigid.curiosity=0.05f; rigid.stability=0.7f;
    Personality flexible{}; flexible.persistence=0.05f; flexible.curiosity=0.95f; flexible.stability=0.7f;
    Personality stable{}; stable.stability=0.95f; stable.boldness=0.8f;
    Personality unstable{}; unstable.stability=0.10f; unstable.boldness=0.2f;
    require(profiles.add(profile(1,"rigid",rigid)),"add rigid profile");
    require(profiles.add(profile(2,"flexible",flexible)),"add flexible profile");
    require(profiles.add(profile(3,"stable",stable)),"add stable profile");
    require(profiles.add(profile(4,"unstable",unstable)),"add unstable profile");
    auto reg=behavior_registry(); auto r=make_runtime(10,reg);
    AgentId ids[4]={{WorldId(1),1},{WorldId(1),2},{WorldId(1),3},{WorldId(1),4}};
    for(int i=0;i<4;++i){ProfileRecipe recipe{};recipe.components={{ProfileId(static_cast<std::uint64_t>(i+1)),1.0f}};
        require(create_agent_from_profiles(*r,ids[i],profiles,recipe,PsychTime(0)),"create profiled agent");}
    Proposition evidence{}; evidence.predicate=PredicateId(1); evidence.name_space=NamespaceId(1);
    for(auto id:ids){ObserveInput o{};o.agent=id;o.proposition=evidence;o.time.process_time=PsychTime(10);o.perceived_certainty=0.8f;o.source_trust=0.8f;require(r->observe(o),"profile evidence");}
    const auto* rigid_state=r->agents().find(ids[0]); const auto* flex_state=r->agents().find(ids[1]);
    require(rigid_state&&flex_state&&!rigid_state->beliefs.empty()&&!flex_state->beliefs.empty(),"profile beliefs exist");
    require(rigid_state->beliefs[0].certainty<flex_state->beliefs[0].certainty,"rigid profile must accept same evidence less readily than flexible profile");

    // Stable agents calm faster than unstable agents from the same episodic fear.
    auto calm=make_runtime(11); AgentId stable_id{WorldId(1),30}, unstable_id{WorldId(1),31};
    AgentInit si{};si.personality=stable;si.emotions={{BasicEmotion::Fear,0.90f,PsychTime(0),PsychTime(0)}};
    AgentInit ui{};ui.personality=unstable;ui.emotions=si.emotions;
    require(calm->create_agent(stable_id,si,PsychTime(0)),"stable calm agent");require(calm->create_agent(unstable_id,ui,PsychTime(0)),"unstable calm agent");
    require(calm->advance_to(stable_id,PsychTime(1000)),"stable advance");require(calm->advance_to(unstable_id,PsychTime(1000)),"unstable advance");
    require(fear_of(*calm->agents().find(stable_id))<fear_of(*calm->agents().find(unstable_id)),"stable profile must calm faster");

    // 2) Hysteresis suppresses oscillation in a near-tie.
    const int with_hysteresis=switch_count(true); const int without_hysteresis=switch_count(false);
    require(with_hysteresis<=10,"hysteresis should keep switch count small");
    require(without_hysteresis>=80,"without continuity near-tie should oscillate frequently");
    require(without_hysteresis>with_hysteresis,"hysteresis must measurably reduce switching");

    // 3) Learning raises/lowers expectations and specializes by context.
    AgentPsychState learned{}; learned.id={WorldId(1),40}; LearningSystem ls;
    for(int i=0;i<8;++i){LearningReport a{};a.action=ActionTypeId(10);a.context_tags={"rain"};a.success=true;a.now=PsychTime(i+1);require(ls.report(learned,a),"learn success");
        LearningReport b{};b.action=ActionTypeId(11);b.context_tags={"rain"};b.success=false;b.now=PsychTime(i+1);require(ls.report(learned,b),"learn failure");}
    const float good=ls.expected_success(learned,ActionTypeId(10),{"rain"}); const float bad=ls.expected_success(learned,ActionTypeId(11),{"rain"});
    require(good>0.75f,"repeated success must raise expectation"); require(bad<0.25f,"repeated failure must lower expectation");
    auto lr=make_runtime(12); AgentInit li{};li.learned_expectations=learned.learned_expectations;AgentId learner{WorldId(1),40};require(lr->create_agent(learner,li,PsychTime(20)),"learning decision agent");
    ActionOption good_action{};good_action.type=ActionTypeId(10);good_action.instance=ActionInstanceId(10);good_action.actor=learner;good_action.context_tags={"rain"};
    ActionOption bad_action=good_action;bad_action.type=ActionTypeId(11);bad_action.instance=ActionInstanceId(11);
    auto ld=lr->decide(learner,{bad_action,good_action},PsychTime(21));require(ld&&ld.value_if()->selected_action==ActionInstanceId(10),"DECIDE must favor learned successful action");
    AgentPsychState contextual{}; contextual.id={WorldId(1),41};
    for(int i=0;i<8;++i){LearningReport rain{};rain.action=ActionTypeId(12);rain.context_tags={"rain"};rain.success=true;rain.now=PsychTime(i+1);require(ls.report(contextual,rain),"rain learning");
        LearningReport sun{};sun.action=ActionTypeId(12);sun.context_tags={"sun"};sun.success=false;sun.now=PsychTime(i+1);require(ls.report(contextual,sun),"sun learning");}
    require(ls.expected_success(contextual,ActionTypeId(12),{"rain"})>ls.expected_success(contextual,ActionTypeId(12),{"sun"})+0.5f,"learning must specialize by context");

    // 4) Contradictory belief evidence and emotional resistance.
    AgentPsychState low{};low.id={WorldId(1),50};low.personality.persistence=0.4f;low.personality.curiosity=0.6f;
    AgentPsychState high=low;high.id={WorldId(1),51}; Proposition fact{};fact.predicate=PredicateId(50);fact.name_space=NamespaceId(1);fact.positive=true;
    BeliefSystem bs;require(bs.add(low,fact,0.90f,0.0f,PsychTime(1)),"low load belief");require(bs.add(high,fact,0.90f,0.95f,PsychTime(1)),"high load belief");
    BeliefEvidence contra{};contra.proposition=fact;contra.proposition.positive=false;contra.strength=0.50f;contra.source_trust=1.0f;contra.observed_at=PsychTime(2);
    require(bs.update(low,contra),"low contradiction");require(bs.update(high,contra),"high contradiction");
    require(high.beliefs[0].certainty>low.beliefs[0].certainty,"emotional load must slow contradictory belief change");
    bool low_flip=!low.beliefs[0].proposition.positive, high_flip=!high.beliefs[0].proposition.positive;
    for(int i=0;i<20&&(!low_flip||!high_flip);++i){contra.observed_at=PsychTime(static_cast<std::uint64_t>(3+i));if(!low_flip){require(bs.update(low,contra),"low contradiction repeat");low_flip=!low.beliefs[0].proposition.positive;}if(!high_flip){require(bs.update(high,contra),"high contradiction repeat");high_flip=!high.beliefs[0].proposition.positive;}}
    require(low_flip&&high_flip,"sufficient contradiction must eventually invert polarity");
    MemoryState memory{};memory.id=MemoryId(1);memory.proposition=fact;memory.importance=0.8f;memory.emotional_load=-0.8f;memory.certainty=0.9f;low.memories.push_back(memory);const Proposition before=low.memories[0].proposition;
    require(ReappraisalSystem{}.reappraise_memory(low,MemoryId(1),0.2f,0.7f),"memory reappraisal");require(equivalent(before,low.memories[0].proposition),"reappraisal must not rewrite factual proposition");

    // 5) Tonic baseline survives decay but is not episodic evidence.
    AgentPsychState tonic{};tonic.id={WorldId(1),60};tonic.personality=Personality{};EmotionSystem es;es.ensure_baseline(tonic,PsychTime(0));
    require(tonic.emotions.size()==6,"all six tonic emotions present");for(const auto&e:tonic.emotions)require(e.intensity>=active_emotion_floor,"tonic emotion floor");
    AgentPsychState empty=tonic;empty.emotions.clear();ActionOption risky{};risky.type=ActionTypeId(20);risky.instance=ActionInstanceId(20);risky.actor=tonic.id;risky.risk=0.6f;
    auto tonic_score=UtilityComposer{}.score(risky,tonic);auto empty_score=UtilityComposer{}.score(risky,empty);require(std::fabs(tonic_score.utility-empty_score.utility)<1.0e-6f,"tonic baseline alone must not alter utility");
    auto br=make_runtime(13);AgentId baseline_agent{WorldId(1),60};require(br->create_agent(baseline_agent,AgentInit{},PsychTime(0)),"baseline runtime agent");auto adv=br->advance_to(baseline_agent,PsychTime(1000000));require(adv&&!adv.value_if()->decision_recommended,"tonic baseline must not trigger decision urgency");
    ActionOption quiet{};quiet.type=ActionTypeId(21);quiet.instance=ActionInstanceId(21);quiet.actor=baseline_agent;auto bd=br->decide(baseline_agent,{quiet},PsychTime(1000001));require(bd&&bd.value_if()->expressed_emotion.empty(),"tonic baseline must not be reported as an expressed episodic emotion");

    // 6) Explanations are arithmetic, not decorative; include hysteresis effects.
    auto er=make_runtime(14);AgentId explain{WorldId(1),70};AgentInit ei{};ei.personality.persistence=0.85f;ei.personality.stability=0.85f;require(er->create_agent(explain,ei,PsychTime(0)),"explanation agent");
    auto first=er->decide(explain,near_tie(explain,true,false),PsychTime(1));require(first,"first explanation decision");check_trace(*first.value_if());
    auto second=er->decide(explain,near_tie(explain,true,true),PsychTime(2));require(second,"hysteresis explanation decision");check_trace(*second.value_if());

    std::cout<<"AUDIT behavior PASS profile_certainty="<<rigid_state->beliefs[0].certainty<<","<<flex_state->beliefs[0].certainty
             <<" switches="<<with_hysteresis<<"/"<<without_hysteresis<<" learning="<<good<<"/"<<bad<<"\n";
}
