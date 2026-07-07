/*
 * Geometry adapter for the TOTEM (GEIGEIGEIST) split — shield totem_left/right
 * on xiao_ble. TOTEM is a 3x5 core PLUS, per hand, a single extra pinky on the
 * bottom row and a 3rd (outer) thumb — 38 physical positions total.
 *
 * Those extras are built from the SHARED add-ons, exactly like the other boards
 * that have them: the bottom extra pinky is the outer column's bottom key
 * (addons/outer_col.h -> Cadet-Shift ( / ) by default), and the 3rd thumb is the
 * shared 3rd thumb (addons/thumb_outer.h -> unused by default). To change either
 * for TOTEM only, #define the fragment before the includes below; to change it for
 * every board that has it, edit the add-on.
 *
 * (Historical note: the shared keymap content was originally transcribed FROM the
 * TOTEM keymap, but that content now lives neutrally in shared/core_blocks.dtsi —
 * TOTEM is a consumer like every other board.)
 *
 * Two artifacts, both derived from this board's matrix-transform order:
 *   1. KEYMAP_LAYER(L) — weaves the shared core + add-ons into TOTEM's physical order.
 *   2. POS_*    — symbolic names for the physical key positions, used by
 *      shared/combos.dtsi and shared/behaviors.dtsi. No raw number lives in shared/.
 *
 * Physical position scheme (matches the board's matrix_transform):
 *    0  1  2  3  4   |   5  6  7  8  9        row 0
 *   10 11 12 13 14   |  15 16 17 18 19        row 1 (home)
 *   20 21 22 23 24 25|  26 27 28 29 30 31     row 2 (20 & 31 = extra pinkies)
 *           32 33 34 |  35 36 37              thumbs (32 & 37 = 3rd/outer thumbs)
 */

#pragma once

/* --- Symbolic key positions (logical core, resolved to TOTEM numbers) --- */
/* row 0 */
#define POS_LT0 0
#define POS_LT1 1
#define POS_LT2 2
#define POS_LT3 3
#define POS_LT4 4
#define POS_RT4 5
#define POS_RT3 6
#define POS_RT2 7
#define POS_RT1 8
#define POS_RT0 9
/* row 1 (home) */
#define POS_LM0 10
#define POS_LM1 11
#define POS_LM2 12
#define POS_LM3 13
#define POS_LM4 14
#define POS_RM4 15
#define POS_RM3 16
#define POS_RM2 17
#define POS_RM1 18
#define POS_RM0 19
/* row 2 (bottom). 20 & 31 are the bottom-row extra pinkies (the outer column's
 * bottom key); they get symbols so combos/HRM could target them. */
#define POS_LOB 20   /* left  bottom extra pinky (outer column, bottom) */
#define POS_LB0 21
#define POS_LB1 22
#define POS_LB2 23
#define POS_LB3 24
#define POS_LB4 25
#define POS_RB4 26
#define POS_RB3 27
#define POS_RB2 28
#define POS_RB1 29
#define POS_RB0 30
#define POS_ROB 31   /* right bottom extra pinky (outer column, bottom) */
/* thumbs. 32 & 37 are the 3rd (outer) thumbs. */
#define POS_LHX 32   /* left  3rd (outer) thumb */
#define POS_LH0 33
#define POS_LH1 34
#define POS_RH1 35
#define POS_RH0 36
#define POS_RHX 37   /* right 3rd (outer) thumb */

/* Shared add-ons: the bottom extra pinky (outer_col.h, bottom row only) and the
 * 3rd thumb (thumb_outer.h). Both are #ifndef-guarded, so a TOTEM-only override
 * would go ABOVE these includes. */
#include "../shared/addons/outer_col.h"
#include "../shared/addons/thumb_outer.h"

/* --- Layout adapter: weave the shared core + add-ons into physical order ---
 * TOP and HOME are pure 3x5 (TOTEM has no top/home outer pinky). The BOTTOM row
 * carries the extra pinky (OUTER_<layer>_bot_*) around the 3x5 core, and the THUMB
 * row carries the 3rd thumb (THUMB_O_<layer>_*) around the inner 2 core thumbs. */
#define KEYMAP_LAYER(L) \
    CORE_##L##_top_L   CORE_##L##_top_R   \
    CORE_##L##_home_L  CORE_##L##_home_R  \
    OUTER_##L##_bot_L  CORE_##L##_bot_L   CORE_##L##_bot_R   OUTER_##L##_bot_R \
    THUMB_O_##L##_L    CORE_##L##_thumb_L CORE_##L##_thumb_R THUMB_O_##L##_R
