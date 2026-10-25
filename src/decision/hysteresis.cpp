#include "mspa/decision/hysteresis.hpp"
#include "mspa/psychology/personality.hpp"
#include <algorithm>
namespace mspa {
float HysteresisPolicy::continuity_bonus(const AgentPsychState&a,const ActionOption&o,ActionContinuityKey current,float urgency)const noexcept{
    if(!current.valid()||o.continuity!=current)return 0;
    const auto p=PersonalitySystem{}.derive(a.personality);
    const float full=0.2f*p.switch_resistance+0.25f*std::clamp(o.progress,0.0f,1.0f)+0.25f*std::clamp(o.interruption_cost,0.0f,1.0f)+0.15f*std::clamp(o.context_stability,0.0f,1.0f);
    // Runtime applies both continuity_bonus and switch_threshold to the incumbent.
    // Each policy term contributes half of its standalone inertia budget so they
    // damp near-tie noise without double-counting enough inertia to lock the agent.
    return 0.5f*full*(1.0f-std::clamp(urgency,0.0f,1.0f));
}
float HysteresisPolicy::switch_threshold(const AgentPsychState&a,const ActionOption&o,float urgency)const noexcept{
    const auto p=PersonalitySystem{}.derive(a.personality);
    const float full=0.05f+0.25f*p.switch_resistance+0.2f*std::clamp(o.interruption_cost,0.0f,1.0f)+0.15f*std::clamp(o.progress,0.0f,1.0f);
    return 0.5f*full*(1.0f-std::clamp(urgency,0.0f,1.0f));
}
}
