/*
 * Geometry adapter for the Le Chiffre BLE ("BLE Chiffre", MangoIV) — a vendored
 * HWv2 nRF52840 unibody board (boards/mangoiv/le_chiff_ble/). 35 physical positions:
 * a 3x5 core + a single CENTER key (in the bottom row's middle) + 2 thumbs/hand,
 * plus a center rotary encoder (a sensor, not a key position).
 *
 * Physical position scheme (from the board's matrix_transform, keymap order):
 *    0  1  2  3  4 |  5  6  7  8  9        row 0 (top)
 *   10 11 12 13 14 | 15 16 17 18 19        row 1 (home)
 *   20 21 22 23 24 [25] 26 27 28 29 30     row 2 (bottom); 25 = center key
 *          31 32   | 33 34                 thumbs (2/hand)
 *
 * The center key (25) is the encoder push -> Mute on base. Note the bottom-right
 * core is 26..30 (shifted by the center), so this can't reuse core_3x5.h.
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
/* row 2 (bottom); 25 = center key */
#define POS_LB0 20
#define POS_LB1 21
#define POS_LB2 22
#define POS_LB3 23
#define POS_LB4 24
#define POS_RB4 26
#define POS_RB3 27
#define POS_RB2 28
#define POS_RB1 29
#define POS_RB0 30
/* thumbs (2/hand) */
#define POS_LH0 31
#define POS_LH1 32
#define POS_RH1 33
#define POS_RH0 34
/* center key = encoder push */
#define POS_CENTER 25

/* Center key: Mute on base, transparent on higher layers. */
#define CHIFF_CENTER_base  &kp C_MUTE
#define CHIFF_CENTER_nav   &trans
#define CHIFF_CENTER_num   &trans
#define CHIFF_CENTER_fun   &trans
#define CHIFF_CENTER_pad   &trans

/* --- Layout adapter: 3x5 core with a center key in the bottom row + 2 thumbs/hand --- */
#define KEYMAP_LAYER(L) \
    CORE_##L##_top_L   CORE_##L##_top_R  \
    CORE_##L##_home_L  CORE_##L##_home_R \
    CORE_##L##_bot_L   CHIFF_CENTER_##L  CORE_##L##_bot_R \
    CORE_##L##_thumb_L CORE_##L##_thumb_R
