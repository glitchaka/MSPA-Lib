#include "test.hpp"
#include "mspa/mspa.hpp"
#include "mspa/psychology/emotions.hpp"
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
using namespace mspa;

namespace {
const char* emotion_name(BasicEmotion e){switch(e){case BasicEmotion::Joy:return"joy";case BasicEmotion::Sadness:return"sadness";case BasicEmotion::Fear:return"fear";case BasicEmotion::Anger:return"anger";case BasicEmotion::Anxiety:return"anxiety";case BasicEmotion::Aversion:return"aversion";}return"unknown";}
RegistrySnapshot make_registry(){
    RegistryBuilder b;
    PredicateSchema seen{};seen.id=PredicateId(1);seen.name_space=NamespaceId(1);seen.default_relevance=0.85f;seen.default_consequence=-0.55f;seen.appraisal_hints={"threat"};
    PredicateSchema danger{};danger.id=PredicateId(2);danger.name_space=NamespaceId(1);danger.default_relevance=0.75f;danger.default_consequence=-0.65f;
    require(b.add_predicate(seen),"trace predicate seen");require(b.add_predicate(danger),"trace predicate danger");
    InferenceRuleData rule{};rule.id=1;rule.premises={{PredicateId(1),true}};rule.conclusion=PredicateId(2);rule.conclusion_namespace=NamespaceId(1);rule.base_certainty=0.80f;
    require(b.add_inference_rule(rule),"trace inference rule");return b.publish();
}
PsychologicalProfile make_profile(std::uint64_t id,const char* name,float persistence,float curiosity,float stability,float boldness,float sociability){PsychologicalProfile p{};p.id=ProfileId(id);p.name=name;p.personality.persistence=persistence;p.personality.curiosity=curiosity;p.personality.stability=stability;p.personality.boldness=boldness;p.personality.sociability=sociability;p.personality.empathy=0.6f;return p;}
std::vector<ActionOption> options(AgentId a){
    ActionOption approach{};approach.type=ActionTypeId(1);approach.instance=ActionInstanceId(1);approach.continuity=ActionContinuityKey(1);approach.actor=a;approach.risk=0.45f;approach.progress=0.80f;approach.novelty=0.40f;
    ActionOption withdraw{};withdraw.type=ActionTypeId(2);withdraw.instance=ActionInstanceId(2);withdraw.continuity=ActionContinuityKey(2);withdraw.actor=a;withdraw.cost=0.08f;withdraw.risk=0.08f;withdraw.progress=0.20f;
    return {approach,withdraw};
}
}

int main(){
    auto registry=make_registry();RuntimeConfig cfg{};cfg.runtime_id=RuntimeId(900);cfg.world_id=WorldId(1);cfg.time_domain={TimeDomainId(1),{60,1}};cfg.seed_root=77;cfg.diagnostics_enabled=true;
    auto rr=Runtime::create(cfg,registry);require(rr,"trace runtime");auto runtime=std::move(*rr.value_if());
    ProfileRegistry profiles;require(profiles.add(make_profile(1,"cautious",0.75f,0.35f,0.45f,0.20f,0.40f)),"trace cautious");require(profiles.add(make_profile(2,"curious",0.30f,0.90f,0.60f,0.65f,0.55f)),"trace curious");require(profiles.add(make_profile(3,"stable-bold",0.55f,0.65f,0.95f,0.90f,0.60f)),"trace stable");require(profiles.add(make_profile(4,"social-sensitive",0.60f,0.55f,0.55f,0.40f,0.95f)),"trace social");
    for(std::uint64_t i=1;i<=4;++i){ProfileRecipe recipe{};recipe.components={{ProfileId(i),1.0f}};AgentId id{WorldId(1),i};require(create_agent_from_profiles(*runtime,id,profiles,recipe,PsychTime(0)),"trace profiled agent");}

    Proposition stimulus{};stimulus.predicate=PredicateId(1);stimulus.name_space=NamespaceId(1);stimulus.positive=true;stimulus.provenance="audit-scenario";
    std::cout<<std::fixed<<std::setprecision(4);
    for(std::uint64_t i=1;i<=4;++i){AgentId id{WorldId(1),i};const auto* prof=profiles.find(ProfileId(i));require(prof,"profile lookup");
        ObserveInput o{};o.agent=id;o.proposition=stimulus;o.time.process_time=PsychTime(10);o.time.has_event_time=true;o.time.event_time=PsychTime(5);o.perceived_certainty=1.0f;o.source_trust=1.0f;
        auto observed=runtime->observe(o);require(observed,"trace observe");
        const auto* state=runtime->agents().find(id);require(state,"trace state");
        std::cout<<"=== PROFILE "<<prof->name<<" agent="<<i<<" ===\n";
        std::cout<<"STIMULUS predicate=1 positive=1 event_time=5 process_time=10\n";
        std::cout<<"PERSONALITY persistence="<<state->personality.persistence<<" curiosity="<<state->personality.curiosity<<" stability="<<state->personality.stability<<" boldness="<<state->personality.boldness<<" sociability="<<state->personality.sociability<<"\n";
        std::cout<<"OBSERVE changed="<<observed.value_if()->state_changed<<" urgency="<<static_cast<int>(observed.value_if()->urgency)<<" decision_recommended="<<observed.value_if()->decision_recommended<<"\n";
        if(const auto* trace=runtime->diagnostics().find(observed.value_if()->diagnostic_token)){for(const auto& item:trace->items)std::cout<<"TRACE stage="<<item.stage<<" detail="<<item.detail<<" weight="<<item.weight<<"\n";}
        bool saw_inference=false;for(const auto& belief:state->beliefs){std::cout<<"BELIEF predicate="<<belief.proposition.predicate.value()<<" positive="<<belief.proposition.positive<<" certainty="<<belief.certainty<<" source="<<static_cast<int>(belief.source)<<"\n";if(belief.proposition.predicate==PredicateId(2)&&belief.source==BeliefSource::Inference)saw_inference=true;}
        require(saw_inference,"trace scenario must include an inferred belief");
        for(const auto& e:state->emotions)std::cout<<"EMOTION name="<<emotion_name(e.kind)<<" intensity="<<e.intensity<<" baseline="<<EmotionSystem{}.baseline(*state,e.kind)<<" excess="<<(e.intensity-EmotionSystem{}.baseline(*state,e.kind))<<"\n";
        std::cout<<"MOOD valence="<<state->mood.valence<<" activation="<<state->mood.activation<<"\n";
        auto decision=runtime->decide(id,options(id),PsychTime(11));require(decision&&decision.value_if()->has_selection,"trace decide");
        for(const auto& candidate:decision.value_if()->trace){std::cout<<"CANDIDATE action="<<candidate.action.value()<<" utility="<<candidate.utility<<"\n";for(const auto& c:candidate.considerations)std::cout<<"  CONSIDERATION name="<<c.name<<" raw="<<c.raw<<" weighted="<<c.weighted<<"\n";}
        std::cout<<"SELECTION action="<<decision.value_if()->selected_action.value()<<" confidence="<<decision.value_if()->confidence<<" expressed_emotion="<<(decision.value_if()->expressed_emotion.empty()?"none":decision.value_if()->expressed_emotion)<<"\n\n";
    }
    std::cout<<"AUDIT trace scenario PASS\n";
}
