/*
 * Geometry adapter for the cb34s shield (nice_nano//zmk). Pure 3x5 + 2-thumbs/side
 * core: 34 physical positions, numbered contiguously, no dead keys. This is the
 * template for any new 3x5+2 board — the LAYOUT() adapter is the identity map.
 *
 * Two artifacts, both derived from this board's matrix-transform order:
 *   1. LAYOUT() — maps the 34 canonical master slots to physical order (identity).
 *   2. POS_*    — symbolic names for the physical key positions.
 *
 * Physical position scheme (matches the board's matrix_transform):
 *    0  1  2  3  4   |   5  6  7  8  9        row 0
 *   10 11 12 13 14   |  15 16 17 18 19        row 1 (home)
 *   20 21 22 23 24   |  25 26 27 28 29        row 2
 *           30 31    |  32 33                 thumbs
 */

#pragma once

/* --- Symbolic key positions (logical core, resolved to cb34s numbers) --- */
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
/* thumbs */
#define POS_LH0 30
#define POS_LH1 31
#define POS_RH1 32
#define POS_RH0 33

/* --- Layout adapter: 90 master slots -> 34 physical positions (pure core) ---
 * Master grid is 6 rows x 7 cols/hand + 3 thumbs/hand (see master_layers.dtsi).
 * cb34s is a pure 3x5+2 core, so it emits only the core slots (identity), ignoring
 * every optional master slot (OUT/INX columns, FN/NUM/EXT rows, TH_O thumbs). */
#define LAYOUT( \
    /* FN   */ s00, s01, s02, s03, s04, s05, s06,   s07, s08, s09, s10, s11, s12, s13, \
    /* NUM  */ s14, s15, s16, s17, s18, s19, s20,   s21, s22, s23, s24, s25, s26, s27, \
    /* TOP  */ s28, s29, s30, s31, s32, s33, s34,   s35, s36, s37, s38, s39, s40, s41, \
    /* HOME */ s42, s43, s44, s45, s46, s47, s48,   s49, s50, s51, s52, s53, s54, s55, \
    /* BOT  */ s56, s57, s58, s59, s60, s61, s62,   s63, s64, s65, s66, s67, s68, s69, \
    /* EXT  */ s70, s71, s72, s73, s74, s75, s76,   s77, s78, s79, s80, s81, s82, s83, \
    /* THMB */ s84, s85, s86,   s87, s88, s89 \
) \
    s29 s30 s31 s32 s33   s36 s37 s38 s39 s40 \
    s43 s44 s45 s46 s47   s50 s51 s52 s53 s54 \
    s57 s58 s59 s60 s61   s64 s65 s66 s67 s68 \
              s85 s86     s87 s88
