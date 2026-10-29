// Cheats from the overlay menu's Cheats tab (runtime/ui/menu.mm). Nothing on disk changes: each cheat writes the
// game's own values in memory, every frame, while it is on.
//
// Infinite stamina: uking::act::PlayerInfo [player_info] +0xa4, the stamina (1000 a wheel; see clock_probe in
// runtime/hle/framepace.c). The game keeps no maximum we know of next to it, so the highest value seen is taken as
// full: the stamina is held there from below, and still rises with a new wheel or a stamina-boosting meal.
//
// Time of day: ksys::world::Manager [world_mgr] (+0x5c0 count, +0x5c8 its managers; [0] = TimeMgr, the time at
// +0xb8, 0..360 = 24 h), held at g_cheat_hour while on, as test_warp_frame does (runtime/services/test_warp.c).
#include <stdint.h>

#include "../game_sites.h"
#include "../hle.h"

#define VF(T, base, off) (*(T *)(uintptr_t)((base) + (off)))

volatile bool g_cheat_stamina, g_cheat_time;
volatile float g_cheat_hour = -1;  // (0..24; -1: not set yet - the menu takes the game's hour when Hold goes on)

static float *time_of_day(void) {
    if (!g_site.world_mgr) return 0;
    uint64_t wm = *(uint64_t *)(uintptr_t)g_site.world_mgr;
    if (!wm || !VF(uint32_t, wm, 0x5c0)) return 0;
    uint64_t mgrs = VF(uint64_t, wm, 0x5c8), tm = mgrs ? *(uint64_t *)(uintptr_t)mgrs : 0;
    return tm ? &VF(float, tm, 0xb8) : 0;
}

// The game's hour now (0..24), or -1 outside a game.
float cheats_game_hour(void) {
    float *t = time_of_day();
    return t ? *t / 15.0f : -1;
}

void cheats_frame(uint64_t frame) {
    (void)frame;
    if (g_cheat_time && g_cheat_hour >= 0) {
        float *t = time_of_day();
        if (t) *t = g_cheat_hour * 15.0f;
    }
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
