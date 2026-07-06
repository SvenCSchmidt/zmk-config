/*
 * Geometry adapter for the Splaytoraid40 (wired RP2040 unibody; vendored shield in
 * boards/shields/splaytoraid40/). 41 physical positions.
 *
 * SUPERSET, but a partial one: an outer pinky column exists only on the TOP and
 * HOME rows (not bottom), and the thumb row is 3 thumbs/hand + a center key.
 *
 * Physical position scheme (from the shield's matrix_transform, keymap order):
 *    0  1  2  3  4  5 |  6  7  8  9 10 11      row 0 (top)   0,11 = outer column
 *   12 13 14 15 16 17 | 18 19 20 21 22 23      row 1 (home)  12,23 = outer column
 *      24 25 26 27 28 | 29 30 31 32 33         row 2 (bottom, NO outer column)
 *         34 35 36  [37]  38 39 40             thumbs; 37 = center (encoder press)
 *
 * Variant A thumbs: the inner 2 thumbs/hand are the core (35,36 left / 38,39 right);
 * the outer thumb (34 left, 40 right) is the shared 3rd thumb (THUMB_O, unused by
 * default). Center 37 = the encoder push (Mute). Encoder rotation is a sensor
 * (sensor-bindings in the keymap), not a key position.
 */

#pragma once

/* --- Core key positions (logical core -> Splaytoraid numbers) --- */
/* row 0 (top) */
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
#define POS_LB0 24
#define POS_LB1 25
#define POS_LB2 26
#define POS_LB3 27
#define POS_LB4 28
#define POS_RB4 29
#define POS_RB3 30
#define POS_RB2 31
#define POS_RB1 32
#define POS_RB0 33
/* core thumbs (inner 2 of the 3-thumb cluster; Variant A) */
#define POS_LH0 35
#define POS_LH1 36
#define POS_RH1 38
#define POS_RH0 39

/* --- Optional extras Splaytoraid physically has (symbols for future use) --- */
#define POS_LOT 0    /* left  outer column, top row  */
#define POS_ROT 11   /* right outer column, top row  */
#define POS_LOH 12   /* left  outer column, home row */
#define POS_ROH 23   /* right outer column, home row */
#define POS_LHX 34   /* left  outer (3rd) thumb      */
#define POS_RHX 40   /* right outer (3rd) thumb      */
#define POS_CENTER 37 /* center key = encoder press  */

/* Outer pinky column + 3rd thumb (shared add-ons; guarded, overridable). The
 * Splaytoraid outer column (Esc/- left, '/\ right) matches the defaults exactly,
 * so no overrides are needed. */
#include "../shared/addons/outer_col.h"
#include "../shared/addons/thumb_outer.h"

/* Center key (encoder push) = Mute on base, transparent on higher layers. */
#define SPLY_CENTER_base  &kp C_MUTE
#define SPLY_CENTER_nav   &trans
#define SPLY_CENTER_num   &trans
#define SPLY_CENTER_fun   &trans
#define SPLY_CENTER_pad   &trans

/* --- Layout adapter: weave the shared core into physical order ---
 * Rows 0-1: outer column (top/home only) + core. Row 2: pure core (no outer).
 * Thumb row: 3rd thumb, 2 core thumbs, center, 2 core thumbs, 3rd thumb. */
#define KEYMAP_LAYER(L) \
    OUTER_##L##_top_L  CORE_##L##_top_L   CORE_##L##_top_R   OUTER_##L##_top_R \
    OUTER_##L##_home_L CORE_##L##_home_L  CORE_##L##_home_R  OUTER_##L##_home_R \
    CORE_##L##_bot_L   CORE_##L##_bot_R \
    THUMB_O_##L##_L CORE_##L##_thumb_L SPLY_CENTER_##L CORE_##L##_thumb_R THUMB_O_##L##_R
