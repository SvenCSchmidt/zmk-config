/*
 * Geometry adapter for the TOTEM (GEIGEIGEIST) split — shield totem_left/right
 * on xiao_ble. 38 physical positions; the outer pinkies (20, 31) and the outer
 * thumbs (32, 37) are unused (&none), leaving a pure 3x5 + 2-thumbs/side core.
 *
 * Two artifacts, both derived from this board's matrix-transform order:
 *   1. LAYOUT() — maps the 34 canonical master slots (see shared/master_layers.dtsi)
 *      to TOTEM's physical bindings order, inserting &none at the dead positions.
 *   2. POS_*    — symbolic names for the physical key positions, used by
 *      shared/combos.dtsi and shared/behaviors.dtsi. No raw number lives in shared/.
 *
 * Physical position scheme (matches the board's matrix_transform):
 *    0  1  2  3  4   |   5  6  7  8  9        row 0
 *   10 11 12 13 14   |  15 16 17 18 19        row 1 (home)
 *   20 21 22 23 24 25|  26 27 28 29 30 31     row 2 (20 & 31 = &none)
 *           32 33 34 |  35 36 37              thumbs (32 & 37 = &none)
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
/* row 2 (bottom). TOTEM physically HAS the outer-pinky keys at 20 & 31 (the
 * master OUT column, bottom row); they get symbols so master edits reach them. */
#define POS_LBX 20   /* left  bottom outer pinky (master OUT column) */
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
#define POS_RBX 31   /* right bottom outer pinky (master OUT column) */
/* thumbs. TOTEM physically HAS the outer thumb keys at 32 & 37 (the master TH_O
 * slots); they get symbols so master edits reach them. */
#define POS_LHX 32   /* left  outer thumb (master TH_O) */
#define POS_LH0 33
#define POS_LH1 34
#define POS_RH1 35
#define POS_RH0 36
#define POS_RHX 37   /* right outer thumb (master TH_O) */

/* --- Layout adapter: 90 master slots -> 38 physical TOTEM positions ---
 * Master grid is 6 rows x 7 cols/hand + 3 thumbs/hand (see master_layers.dtsi).
 * Besides the 3x5+2 core, TOTEM physically has the outer-pinky keys (master OUT
 * column, bottom row -> s56/s69) and the outer thumbs (master TH_O -> s84/s89),
 * so those master slots ARE emitted: populating them in master_layers.dtsi
 * automatically reaches TOTEM. The other optional slots (INX column, FN/NUM/EXT
 * rows, the OUT column on non-bottom rows) are not physical on TOTEM and dropped. */
#define LAYOUT( \
    /* FN   */ s00, s01, s02, s03, s04, s05, s06,   s07, s08, s09, s10, s11, s12, s13, \
    /* NUM  */ s14, s15, s16, s17, s18, s19, s20,   s21, s22, s23, s24, s25, s26, s27, \
    /* TOP  */ s28, s29, s30, s31, s32, s33, s34,   s35, s36, s37, s38, s39, s40, s41, \
    /* HOME */ s42, s43, s44, s45, s46, s47, s48,   s49, s50, s51, s52, s53, s54, s55, \
    /* BOT  */ s56, s57, s58, s59, s60, s61, s62,   s63, s64, s65, s66, s67, s68, s69, \
    /* EXT  */ s70, s71, s72, s73, s74, s75, s76,   s77, s78, s79, s80, s81, s82, s83, \
    /* THMB */ s84, s85, s86,   s87, s88, s89 \
) \
        s29 s30 s31 s32 s33   s36 s37 s38 s39 s40     \
        s43 s44 s45 s46 s47   s50 s51 s52 s53 s54     \
    s56 s57 s58 s59 s60 s61   s64 s65 s66 s67 s68 s69 \
              s84 s85 s86     s87 s88 s89
