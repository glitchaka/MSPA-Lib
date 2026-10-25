#include <stdint.h>
#include "mspa/c_api.h"

int main(void) {
    if (mspa_version_major() != 1u) return 1;
    mspa_runtime_config config = {0};
    config.runtime_id = 1;
    config.world_id = 1;
    config.time_domain_id = 1;
    config.ticks_per_second_num = 1;
    config.ticks_per_second_den = 1;
    config.seed_root = 7;
    config.diagnostics_enabled = 0;
    mspa_runtime* runtime = 0;
    if (mspa_runtime_create(&config, &runtime) != MSPA_OK || runtime == 0) return 2;
    mspa_agent_id agent = {1, 1};
    if (mspa_agent_create(runtime, agent, 0) != MSPA_OK) { mspa_runtime_destroy(runtime); return 3; }
    if (!mspa_agent_has(runtime, agent)) { mspa_runtime_destroy(runtime); return 4; }
    mspa_runtime_destroy(runtime);
    return 0;
}
