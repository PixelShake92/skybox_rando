#include "modding.h"
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "recompconfig.h"

#define SKY_STAGE_COUNT  9
#define SKY_SOURCE_COUNT 12

#define SKY_CHOICE_VANILLA 0
#define SKY_CHOICE_RANDOM  1
#define SKY_SOURCE_BASE    2

typedef struct {
    s16 model_id;
    f32 scale;
    f32 rotation_speed;
} SkyInfo;

typedef struct {
    s16 map;
    SkyInfo sky_list[3];
} MapSkyInfo;

typedef struct {
    s16 map;
    const char *config_id;
} SkyStage;

// Vanilla map to sky table
extern MapSkyInfo D_8036BD40[];

s32 globalTimer_getTime(void);

// Maps that get a config selection. Every other entry keeps its vanilla sky.
SkyStage sky_stages[SKY_STAGE_COUNT] = {
    {MAP_1_SM_SPIRAL_MOUNTAIN,       "sky_sm"},
    {MAP_2_MM_MUMBOS_MOUNTAIN,       "sky_mm"},
    {MAP_C_MM_TICKERS_TOWER,         "sky_tickers"},
    {MAP_7_TTC_TREASURE_TROVE_COVE,  "sky_ttc"},
    {MAP_12_GV_GOBIS_VALLEY,         "sky_gv"},
    {MAP_1B_MMM_MAD_MONSTER_MANSION, "sky_mmm"},
    {MAP_27_FP_FREEZEEZY_PEAK,       "sky_fp"},
    {MAP_31_RBB_RUSTY_BUCKET_BAY,    "sky_rbb"},
    {MAP_75_GL_MMM_LOBBY,            "sky_lobby"}
};

// Maps whose sky rows can be borrowed. Order matches the selection options after
// Vanilla and Random.
s16 sky_source_maps[SKY_SOURCE_COUNT] = {
    MAP_1_SM_SPIRAL_MOUNTAIN,
    MAP_2_MM_MUMBOS_MOUNTAIN,
    MAP_C_MM_TICKERS_TOWER,
    MAP_7_TTC_TREASURE_TROVE_COVE,
    MAP_12_GV_GOBIS_VALLEY,
    MAP_1B_MMM_MAD_MONSTER_MANSION,
    MAP_27_FP_FREEZEEZY_PEAK,
    MAP_31_RBB_RUSTY_BUCKET_BAY,
    MAP_75_GL_MMM_LOBBY,
    MAP_87_CS_SPIRAL_MOUNTAIN_5,
    MAP_95_CS_END_ALL_100,
    MAP_1F_CS_START_RAREWARE
};

u32 sky_random_state = 0;

u32 skyRandomNext(void) {
    u32 x;

    if (sky_random_state == 0) {
        sky_random_state = (u32)globalTimer_getTime();
        if (sky_random_state == 0) {
            sky_random_state = 0x9E3779B9;
        }
    }

    x = sky_random_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    sky_random_state = x;
    return x;
}

s32 skyStageIndex(s16 map_id) {
    s32 i;

    for (i = 0; i < SKY_STAGE_COUNT; i++) {
        if (sky_stages[i].map == map_id) {
            return i;
        }
    }
    return -1;
}

// Resolves the map to the sky selected for it, then runs the vanilla scan. The
// whole SkyInfo row travels together.
RECOMP_PATCH MapSkyInfo *sky_getMapSkyInfo(enum map_e map_id) {
    MapSkyInfo *entry = D_8036BD40;
    s16 lookup = (s16)map_id;
    s32 stage = skyStageIndex(lookup);
    u32 choice;

    if (stage >= 0) {
        choice = (u32)recomp_get_config_u32(sky_stages[stage].config_id);
        if (choice == SKY_CHOICE_RANDOM) {
            lookup = sky_source_maps[skyRandomNext() % SKY_SOURCE_COUNT];
        } else if (choice != SKY_CHOICE_VANILLA) {
            lookup = sky_source_maps[choice - SKY_SOURCE_BASE];
        }
    }

    while (entry->map) {
        if (lookup == entry->map) {
            return entry;
        }
        entry++;
    }
    return entry;
}
