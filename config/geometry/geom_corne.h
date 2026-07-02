/*
 * Geometry adapter for the Corne (foostan) 6-column layout — the standard corne
 * shield built into ZMK core (corne_left / corne_right on nice_nano). This is the
 * first SUPERSET: 3x6 + 3 thumbs/side = 42 physical positions. Beyond the shared
 * 3x5+2 core it has an outer pinky column (all 3 rows) and a 3rd (outer) thumb.
 *
 * Those extras are mapped to the master's optional OUT-column and TH_O slots, so
 * populating them in master_layers.dtsi automatically reaches Corne. They are
 * currently reserved (&none on base / &trans on higher layers) -> inactive for now.
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

/* --- Layout adapter: 90 master slots -> 42 physical Corne positions ---
 * Emits the core plus the outer column (master OUT slots) and 3rd thumb (TH_O). */
#define LAYOUT( \
    /* FN   */ s00, s01, s02, s03, s04, s05, s06,   s07, s08, s09, s10, s11, s12, s13, \
    /* NUM  */ s14, s15, s16, s17, s18, s19, s20,   s21, s22, s23, s24, s25, s26, s27, \
    /* TOP  */ s28, s29, s30, s31, s32, s33, s34,   s35, s36, s37, s38, s39, s40, s41, \
    /* HOME */ s42, s43, s44, s45, s46, s47, s48,   s49, s50, s51, s52, s53, s54, s55, \
    /* BOT  */ s56, s57, s58, s59, s60, s61, s62,   s63, s64, s65, s66, s67, s68, s69, \
    /* EXT  */ s70, s71, s72, s73, s74, s75, s76,   s77, s78, s79, s80, s81, s82, s83, \
    /* THMB */ s84, s85, s86,   s87, s88, s89 \
) \
    s28 s29 s30 s31 s32 s33   s36 s37 s38 s39 s40 s41 \
    s42 s43 s44 s45 s46 s47   s50 s51 s52 s53 s54 s55 \
    s56 s57 s58 s59 s60 s61   s64 s65 s66 s67 s68 s69 \
            s84 s85 s86       s87 s88 s89
