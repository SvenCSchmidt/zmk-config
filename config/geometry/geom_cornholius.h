/*
 * Geometry adapter for the Cornholius (ebastler) — a wireless nRF52840 unibody
 * "drop-in" PCB for the Foostan Cornelius. Vendored HWv2 board lives in
 * boards/ebastler/cornholius/; this header maps the shared master onto its matrix.
 *
 * SUPERSET (larger than Corne): 4 rows x 12 columns = 48 physical positions. Beyond
 * the shared 3x5+2 core it has an outer pinky column on all three alpha rows AND a
 * full fourth row (12 keys) that carries the 6-key thumb cluster plus outer
 * modifier keys. Rows 0-2 are numbered exactly like Corne; only row 3 is wider.
 *
 * The core 3x5 alphas + inner 2 thumbs/hand map to the shared core slots. Every
 * extra (outer column, fourth-row outer keys, outer thumbs) maps to a reserved
 * master slot (OUT columns, EXT row, TH_O thumbs) — currently &none/&trans, i.e.
 * inactive, matching Corne. Populating those master slots later reaches Cornholius.
 *
 * Physical position scheme (cornholius_transform, 12 cols x 4 rows):
 *    0  1  2  3  4  5 |  6  7  8  9 10 11      row 0 (top)    (0 & 11 = outer col)
 *   12 13 14 15 16 17 | 18 19 20 21 22 23      row 1 (home)   (12 & 23 = outer col)
 *   24 25 26 27 28 29 | 30 31 32 33 34 35      row 2 (bottom) (24 & 35 = outer col)
 *   36 37 38 39 40 41 | 42 43 44 45 46 47      row 3 (thumb/4th)
 *      -> 36 37 38 / 45 46 47 = outer modifier keys; 39..44 = 6-thumb cluster
 */

#pragma once

/* --- Core key positions (logical core -> Cornholius inner-5-cols / inner-2-thumbs) --- */
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
/* core thumbs (inner 2 of the 6-thumb cluster) */
#define POS_LH0 40
#define POS_LH1 41
#define POS_RH1 42
#define POS_RH0 43

/* --- Optional extras Cornholius physically has (reserved; symbols for future use) --- */
#define POS_LOT 0    /* left  outer column, top row     */
#define POS_LOH 12   /* left  outer column, home row    */
#define POS_LOB 24   /* left  outer column, bottom row  */
#define POS_ROT 11   /* right outer column, top row     */
#define POS_ROH 23   /* right outer column, home row    */
#define POS_ROB 35   /* right outer column, bottom row  */
#define POS_LHX 39   /* left  outer thumb (TH_O)        */
#define POS_RHX 44   /* right outer thumb (TH_O)        */

/* --- Outer pinky column: shared add-on, with Cornholius' three overrides ---
 * Cornholius' outer column differs from the default (outer_col.h) only in three
 * base positions, so we override those BEFORE including the add-on (guarded ->
 * our value wins), and inherit the rest (top-L Esc, bottom Cadet-Shift both hands,
 * higher layers transparent). */
#define OUTER_base_home_L  &kp TAB      /* Cornholius: Tab (default was  - ) */
#define OUTER_base_top_R   &kp BSPC     /* Cornholius: Backspace (default was ') */
#define OUTER_base_home_R  &kp RET      /* Cornholius: Enter (default was  \ ) */
#include "../shared/addons/outer_col.h"

/* --- 4th-row outer modifier keys (Cornholius-specific) ---
 * The lower row's outer triplets: left = Ctrl/Win/Alt, right (physical L->R) =
 * AltGr/Win/Ctrl on base; transparent on the higher layers. */
#define C_R3_TRANS   &trans &trans &trans
#define C_R3_base_L  &kp LCTRL &kp LGUI &kp LALT
#define C_R3_base_R  &kp RALT  &kp RGUI &kp RCTRL
#define C_R3_nav_L   C_R3_TRANS
#define C_R3_nav_R   C_R3_TRANS
#define C_R3_num_L   C_R3_TRANS
#define C_R3_num_R   C_R3_TRANS
#define C_R3_fun_L   C_R3_TRANS
#define C_R3_fun_R   C_R3_TRANS
#define C_R3_pad_L   C_R3_TRANS
#define C_R3_pad_R   C_R3_TRANS

/* --- Layout adapter: weave the shared core into physical order ---
 * Rows 0-2: outer pinky column (OUTER_*) + 3x5 core. Row 3 left-to-right:
 * 3 outer modifiers (C_R3_L) + outer thumb (RSVD/free) + 2 core thumbs (L) +
 * 2 core thumbs (R) + outer thumb (RSVD/free) + 3 outer modifiers (C_R3_R). */
#define KEYMAP_LAYER(L) \
    OUTER_##L##_top_L  CORE_##L##_top_L   CORE_##L##_top_R   OUTER_##L##_top_R \
    OUTER_##L##_home_L CORE_##L##_home_L  CORE_##L##_home_R  OUTER_##L##_home_R \
    OUTER_##L##_bot_L  CORE_##L##_bot_L   CORE_##L##_bot_R   OUTER_##L##_bot_R \
    C_R3_##L##_L RSVD_##L CORE_##L##_thumb_L CORE_##L##_thumb_R RSVD_##L C_R3_##L##_R
