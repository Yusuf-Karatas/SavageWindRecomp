// Cheats from the overlay menu's Cheats tab (runtime/ui/menu.mm). Nothing on disk changes: each cheat writes the
// game's own values in memory, every frame, while it is on.
//
// Infinite stamina: uking::act::PlayerInfo [player_info] +0xa4, the stamina (1000 a wheel; see clock_probe in
// runtime/hle/framepace.c). The game keeps no maximum we know of next to it, so the highest value seen is taken as
// full: the stamina is held there from below, and still rises with a new wheel or a stamina-boosting meal.
//
// Time of day: ksys::world::Manager [world_mgr] (+0x5c0 count, +0x5c8 its managers; [0] = TimeMgr, the time at
// +0xb8, 0..360 = 24 h), held at g_cheat_hour while on, as test_warp_frame does (runtime/services/test_warp.c).
//
// Infinite health: PlayerInfo +0x60 is the player actor and +0xa0 the heart containers in quarter hearts (f32; the
// BotW decompilation's PlayerInfo layout). PlayerInfo::getLifeFromPlayerActor calls the actor's getLife, vtable +0x2b8
// (1.9.0: 0x7000cc7184), which returns the life's address: `add x0, x0, #off; ret`. That body is read from the game's
// code each time the actor changes, so the life (s32, quarter hearts) is only written when it is exactly that.
//
// Weather (test): WeatherMgr is world::Manager's managers[3]; its forecasts, 20 climates x 3 time blocks (s32 each), at
// +0x194. They are only written while every one of them reads as a weather (0..8), so a different layout is left alone.
// The weather follows the forecast at the next change of time block, not at once.
//
// Infinite arrows (1.9.0 only: the singleton below was found in its main module; tools/game_sites.py doesn't look for it):
// - uking::ui::PauseMenuDataMgr [0x7001e18570], the inventory: its item list at +0x68 (sead::OffsetList: next node
//   +0x70, count +0x78, node offset +0x7c), as setEquippedWeaponItemValue walks it. PouchItem: +0x18 type (2 arrows),
//   +0x20 value (count or durability), +0x24 equipped, +0x30 the name's characters (FixedSafeString<64> at +0x28).
#include <stdint.h>
#include <string.h>

#include "../game_sites.h"
#include "../hle.h"
#include "../kernel.h"

#define VF(T, base, off) (*(T *)(uintptr_t)((base) + (off)))

volatile bool g_cheat_stamina, g_cheat_time, g_cheat_health, g_cheat_weather;
volatile int g_cheat_weather_type;  // (WeatherType: 0 clear, 1 cloudy, 2 rain, 3 heavy rain, 4 snow, 5 heavy snow,
                                    // 6 thunderstorm, 7 thunder rain)
volatile int g_cheat_health_state;  // 0 not looked yet, 1 found, -1 the game's code isn't what this expects
volatile bool g_cheat_arrows;
volatile float g_cheat_hour = -1;  // (0..24; -1: not set yet - the menu takes the game's hour when Hold goes on)

// world::Manager's managers[i] (0 TimeMgr, 3 WeatherMgr), or 0
static uint64_t world_job(uint32_t i) {
    if (!g_site.world_mgr) return 0;
    uint64_t wm = *(uint64_t *)(uintptr_t)g_site.world_mgr;
    if (!wm || VF(uint32_t, wm, 0x5c0) <= i) return 0;
    uint64_t mgrs = VF(uint64_t, wm, 0x5c8);
    return mgrs ? VF(uint64_t, mgrs, 8 * i) : 0;
}

static float *time_of_day(void) {
    uint64_t tm = world_job(0);
    return tm ? &VF(float, tm, 0xb8) : 0;
}

static void weather_frame(void) {
    uint64_t wx = world_job(3);
    if (!wx) return;
    int32_t *f = &VF(int32_t, wx, 0x194);
    for (int i = 0; i < 60; i++)
        if (f[i] < 0 || f[i] > 8) return;
    for (int i = 0; i < 60; i++) f[i] = g_cheat_weather_type;
}

// The player actor's life (quarter hearts), or 0
static int32_t *player_life(uint64_t actor) {
    static uint64_t last_vt, off;
    if (!actor) return 0;
    uint64_t vt = VF(uint64_t, actor, 0);
    if (vt != last_vt) {
        last_vt = vt, off = 0;
        uint64_t fn = vt ? VF(uint64_t, vt, 0x2b8) : 0;
        // (the main module's code: from its base up to its .data)
        if (fn >= 0x7000000000ull && fn < g_site.main_data && !(fn & 3)) {
            uint32_t add = VF(uint32_t, fn, 0), ret = VF(uint32_t, fn, 4);
            if ((add & 0xff8003ff) == 0x91000000 && ret == 0xd65f03c0)  // add x0, x0, #imm{, lsl #12}; ret
                off = (uint64_t)((add >> 10) & 0xfff) << (add & 0x400000 ? 12 : 0);
        }
        g_cheat_health_state = off ? 1 : -1;
    }
    return off ? &VF(int32_t, actor, off) : 0;
}

// Guest memory that is there: the main module, or the guest heap as far as the game has set it
static bool mem_ok(uint64_t a, uint64_t n) {
    if (a >= 0x7000000000ull && a + n <= g_site.main_end) return true;
    return a >= HEAP_BASE && a + n <= HEAP_BASE + kernel_heap_size();
}

static bool v190(void) { return g_game_version && !strcmp(g_game_version, "1.9.0"); }

// Each inventory item in turn (f returns false to stop)
static void pouch_each(bool (*f)(uint64_t item, void *ctx), void *ctx) {
    if (!v190() || !mem_ok(0x7001e18570ull, 8)) return;
    uint64_t mgr = VF(uint64_t, 0x7001e18570ull, 0);
    if (!mem_ok(mgr + 0x68, 0x18)) return;
    int32_t off = VF(int32_t, mgr, 0x7c), n = VF(int32_t, mgr, 0x78);
    if (off < 0 || off > 0x100 || n < 0 || n > 1000) return;
    uint64_t head = mgr + 0x68, node = VF(uint64_t, mgr, 0x70);
    for (int i = 0; i < n && node != head && mem_ok(node, 0x10); i++) {
        uint64_t item = node - off;
        if (!mem_ok(item, 0x80) || !f(item, ctx)) return;
        node = VF(uint64_t, node, 8);
    }
}

typedef struct { uint64_t item; int32_t held; } Held;
static Held held_arrows[16];

static bool arrow_item(uint64_t item, void *ctx) {
    (void)ctx;
    if (VF(int32_t, item, 0x18) != 2) return true;  // (PouchItemType::Arrow)
    int32_t *v = &VF(int32_t, item, 0x20);
    Held *h = 0, *free_slot = 0;
    for (int i = 0; i < 16; i++) {
        if (held_arrows[i].item == item) h = &held_arrows[i];
        else if (!held_arrows[i].item && !free_slot) free_slot = &held_arrows[i];
    }
    if (!h && free_slot) *free_slot = (Held){item, *v}, h = free_slot;
    if (!h) return true;
    if (*v > h->held) h->held = *v;
    else if (*v < h->held && *v >= 0) *v = h->held;
    return true;
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
    if (g_cheat_weather) weather_frame();
    if (g_cheat_arrows) pouch_each(arrow_item, 0);
    else memset(held_arrows, 0, sizeof held_arrows);
    static uint64_t last_pinfo;
    static float full;
    if (!g_site.player_info) return;  // (no game code loaded yet)
    uint64_t pinfo = *(uint64_t *)(uintptr_t)g_site.player_info;
    if (pinfo != last_pinfo) last_pinfo = pinfo, full = 0;  // (another game loaded: learn its full stamina again)
    if (!pinfo) return;
    float *stamina = &VF(float, pinfo, 0xa4);
    if (*stamina > full) full = *stamina;
    if (g_cheat_stamina && full > 0 && *stamina < full) *stamina = full;
    if (g_cheat_health) {
        int32_t *life = player_life(VF(uint64_t, pinfo, 0x60));
        int32_t max = (int32_t)VF(float, pinfo, 0xa0);
        if (life && max > 0 && max <= 40 * 4 && *life > 0 && *life < max) *life = max;
    }
}
