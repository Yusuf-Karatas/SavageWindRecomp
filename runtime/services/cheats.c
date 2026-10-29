// Cheats from the overlay menu's Cheats tab (runtime/ui/menu.mm). Nothing on disk changes: each cheat writes the
// game's own values in memory, every frame, while it is on.
//
// Infinite stamina: uking::act::PlayerInfo [player_info] +0xa4, the stamina (1000 a wheel; see clock_probe in
// runtime/hle/framepace.c). The game keeps no maximum we know of next to it, so the highest value seen is taken as
// full: the stamina is held there from below, and still rises with a new wheel or a stamina-boosting meal.
#include <stdint.h>

#include "../game_sites.h"
#include "../hle.h"

#define VF(T, base, off) (*(T *)(uintptr_t)((base) + (off)))

volatile bool g_cheat_stamina;

void cheats_frame(uint64_t frame) {
    (void)frame;
    static uint64_t last_pinfo;
    static float full;
    if (!g_site.player_info) return;  // (no game code loaded yet)
    uint64_t pinfo = *(uint64_t *)(uintptr_t)g_site.player_info;
    if (pinfo != last_pinfo) last_pinfo = pinfo, full = 0;  // (another game loaded: learn its full stamina again)
    if (!pinfo) return;
    float *stamina = &VF(float, pinfo, 0xa4);
    if (*stamina > full) full = *stamina;
    if (g_cheat_stamina && full > 0 && *stamina < full) *stamina = full;
}
