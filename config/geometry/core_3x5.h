/*
 * core_3x5.h — the shared 3x5 alpha key positions, numbered CONTIGUOUSLY 0..29
 * (row-major, both hands per row). Every board whose alpha core is wired this way
 * includes this instead of repeating the 30 POS_* lines; the includer then adds its
 * own thumb positions (which differ per board) and its KEYMAP_LAYER weave.
 *
 *    0  1  2  3  4 |  5  6  7  8  9        row 0 (top)
 *   10 11 12 13 14 | 15 16 17 18 19        row 1 (home)
 *   20 21 22 23 24 | 25 26 27 28 29        row 2 (bottom)
 *
 * The alpha content itself lives once in config/shared/core_blocks.dtsi; this file
 * only carries the position NUMBERS the combos/HRMs reference.
 */

#pragma once

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
