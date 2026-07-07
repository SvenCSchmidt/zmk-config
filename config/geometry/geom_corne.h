/*
 * Geometry adapter for the Corne (foostan) 6-column layout — the standard corne
 * shield built into ZMK core (corne_left / corne_right on nice_nano). This is the
 * first SUPERSET: 3x6 + 3 thumbs/side = 42 physical positions. Beyond the shared
 * 3x5+2 core it has an outer pinky column (all 3 rows) and a 3rd (outer) thumb.
 *
 * The outer pinky column is populated from the shared add-on config/shared/addons/
 * outer_col.h (Esc / - / Cadet-( on the left, ' / \ / Cadet-) on the right; higher
 * layers transparent). The 3rd (outer) thumb uses the shared thumb_outer.h add-on
 * (unused by default), so it stays reserved for now.
 *
 * Physical position scheme (corne default_transform, 12 cols x 4 rows):
 *    0  1  2  3  4  5 |  6  7  8  9 10 11      row 0   (0 & 11 = outer pinkies)
 *   12 13 14 15 16 17 | 18 19 20 21 22 23      row 1 (home)
 *   24 25 26 27 28 29 | 30 31 32 33 34 35      row 2
 *         36 37 38    | 39 40 41               thumbs  (36 & 41 = outer thumbs)
 */

#pragma once

/* --- Core key positions (logical core -> corne inner-5-columns/inner-2-thumbs) --- */
/* row 0 */
#define POS_LT0 1
#define POS_LT1 2
#define POS_LT2 3
#define POS_LT3 4
#define POS_LT4 5
#define POS_RT4 6
#define POS_RT3 7
#define POS_RT2 8
#define POS_RT1 9
#define POS_RT0 10
/* row 1 (home) */
#define POS_LM0 13
#define POS_LM1 14
#define POS_LM2 15
#define POS_LM3 16
#define POS_LM4 17
#define POS_RM4 18
#define POS_RM3 19
#define POS_RM2 20
#define POS_RM1 21
#define POS_RM0 22
/* row 2 (bottom) */
#define POS_LB0 25
#define POS_LB1 26
#define POS_LB2 27
#define POS_LB3 28
#define POS_LB4 29
#define POS_RB4 30
#define POS_RB3 31
#define POS_RB2 32
#define POS_RB1 33
#define POS_RB0 34
/* core thumbs (inner 2 of the 3-thumb cluster) */
#define POS_LH0 37
#define POS_LH1 38
#define POS_RH1 39
#define POS_RH0 40

/* --- Optional extras Corne physically has (reserved; symbols for future use) --- */
#define POS_LOT 0    /* left  outer column, top row    */
#define POS_LOH 12   /* left  outer column, home row   */
#define POS_LOB 24   /* left  outer column, bottom row */
#define POS_ROT 11   /* right outer column, top row    */
#define POS_ROH 23   /* right outer column, home row   */
#define POS_ROB 35   /* right outer column, bottom row */
#define POS_LHX 36   /* left  outer thumb (TH_O)  */
#define POS_RHX 41   /* right outer thumb (TH_O)  */

/* Outer pinky column + 3rd thumb (guarded; a board could override before include). */
#include "../shared/addons/outer_col.h"
#include "../shared/addons/thumb_outer.h"

/* --- Layout adapter: weave the shared core into physical order ---
 * Outer pinky column (top/home/bottom, both hands) = OUTER_<layer>_* from the
 * add-on; the 3x5 core = CORE_<layer>_*; the 3rd (outer) thumb per hand =
 * THUMB_O_<layer>_* (shared add-on, unused by default). */
#define KEYMAP_LAYER(L) \
    OUTER_##L##_top_L   CORE_##L##_top_L   CORE_##L##_top_R   OUTER_##L##_top_R \
    OUTER_##L##_home_L  CORE_##L##_home_L  CORE_##L##_home_R  OUTER_##L##_home_R \
    OUTER_##L##_bot_L   CORE_##L##_bot_L   CORE_##L##_bot_R   OUTER_##L##_bot_R \
    THUMB_O_##L##_L     CORE_##L##_thumb_L CORE_##L##_thumb_R THUMB_O_##L##_R
