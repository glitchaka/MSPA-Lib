#include <stdint.h>
#include "mspa/c_api.h"
int main(void){
    if(mspa_version_major()!=1u||mspa_version_minor()<1u)return 1;
    mspa_runtime_config config={0};config.runtime_id=1;config.world_id=1;config.time_domain_id=1;config.ticks_per_second_num=1;config.ticks_per_second_den=1;config.seed_root=7;config.diagnostics_enabled=0;
    mspa_runtime*runtime=0;if(mspa_runtime_create(&config,&runtime)!=MSPA_OK||runtime==0)return 2;
    mspa_agent_id agent={1,1};if(mspa_agent_create(runtime,agent,0)!=MSPA_OK){mspa_runtime_destroy(runtime);return 3;}if(!mspa_agent_has(runtime,agent)){mspa_runtime_destroy(runtime);return 4;}
    mspa_profile_registry*profiles=0;if(mspa_profile_registry_create(1,&profiles)!=MSPA_OK||profiles==0){mspa_runtime_destroy(runtime);return 5;}if(mspa_profile_count(profiles)!=3){mspa_profile_registry_destroy(profiles);mspa_runtime_destroy(runtime);return 6;}
    mspa_profile_weight weights[2]={{mspa_profile_builtin_cautious_persistent(),1.0f},{mspa_profile_builtin_curious_bold(),1.0f}};mspa_profile_recipe recipe={weights,2,99,0.05f};mspa_agent_id profiled={1,2};if(mspa_agent_create_profiled(runtime,profiled,profiles,&recipe,1)!=MSPA_OK){mspa_profile_registry_destroy(profiles);mspa_runtime_destroy(runtime);return 7;}if(!mspa_agent_has(runtime,profiled)){mspa_profile_registry_destroy(profiles);mspa_runtime_destroy(runtime);return 8;}
    mspa_profile_definition custom={0};custom.profile_id=100;custom.name="custom";custom.personality.sociability=0.9f;custom.personality.stability=0.5f;custom.personality.empathy=0.5f;custom.personality.persistence=0.5f;custom.personality.curiosity=0.5f;custom.personality.boldness=0.5f;if(mspa_profile_register(profiles,&custom)!=MSPA_OK||mspa_profile_count(profiles)!=4){mspa_profile_registry_destroy(profiles);mspa_runtime_destroy(runtime);return 9;}
    mspa_profile_registry_destroy(profiles);mspa_runtime_destroy(runtime);return 0;
}
