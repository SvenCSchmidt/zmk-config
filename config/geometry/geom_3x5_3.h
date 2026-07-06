/*
 * geom_3x5_3.h — shared geometry adapter for any board whose physical matrix is a
 * 3x5 core + 3 thumbs/hand numbered CONTIGUOUSLY 0..35 (row-major, both hands per
 * row; thumbs 30/31/32 left, 33/34/35 right). No outer column, no center key.
 * Analogous to geom_3x5_2.h, one thumb wider. Boards using it: MNHTTN, le_oeuf.
 *
 * Physical position scheme:
 *    0  1  2  3  4 |  5  6  7  8  9        row 0 (top)
 *   10 11 12 13 14 | 15 16 17 18 19        row 1 (home)
 *   20 21 22 23 24 | 25 26 27 28 29        row 2 (bottom)
 *       30 31 32   | 33 34 35              thumbs (3/hand)
 *
 * Variant A thumbs: inner 2/hand are the core (31,32 left / 33,34 right); the outer
 * thumb (30 left, 35 right) is the shared 3rd thumb (addons/thumb_outer.h).
 */

#pragma once

/* --- Core key positions --- */
/* row 0 (top) */
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
/* row 2 (bottom) */
#define POS_LB0 20
#define POS_LB1 21
#define POS_LB2 22
#define POS_LB3 23
#define POS_LB4 24
#define POS_RB4 25
#define POS_RB3 26
#define POS_RB2 27
#define POS_RB1 28
#define POS_RB0 29
/* core thumbs (inner 2 of the 3-thumb cluster; Variant A) */
#define POS_LH0 31
#define POS_LH1 32
#define POS_RH1 33
#define POS_RH0 34
/* extras */
#define POS_LHX 30   /* left  outer (3rd) thumb  */
#define POS_RHX 35   /* right outer (3rd) thumb  */

/* 3rd thumb (shared add-on; no outer column, no center key on these boards). */
#include "../shared/addons/thumb_outer.h"

/* --- Layout adapter: pure 3x5 core + 3-thumb row (Variant A) --- */
#define KEYMAP_LAYER(L) \
    CORE_##L##_top_L   CORE_##L##_top_R  \
    CORE_##L##_home_L  CORE_##L##_home_R \
    CORE_##L##_bot_L   CORE_##L##_bot_R  \
    THUMB_O_##L##_L CORE_##L##_thumb_L CORE_##L##_thumb_R THUMB_O_##L##_R
