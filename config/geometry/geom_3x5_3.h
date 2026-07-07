/*
 * geom_3x5_3.h — shared geometry adapter for any board whose physical matrix is a
 * 3x5 core + 3 thumbs/hand numbered CONTIGUOUSLY 0..35 (thumbs 30/31/32 left,
 * 33/34/35 right). Same as geom_3x5_2.h, one thumb wider. No outer column, no center.
 *
 *    0  1  2  3  4 |  5  6  7  8  9        row 0 (top)
 *   10 11 12 13 14 | 15 16 17 18 19        row 1 (home)
 *   20 21 22 23 24 | 25 26 27 28 29        row 2 (bottom)
 *       30 31 32   | 33 34 35              thumbs (3/hand)
 *
 * The 30 alpha positions come from the shared core_3x5.h; this file only adds the
 * thumb arrangement. Variant A: inner 2/hand are the core (31,32 left / 33,34 right);
 * the outer thumb (30 left, 35 right) is the shared 3rd thumb (addons/thumb_outer.h).
 * Boards using it: MNHTTN, Onyx Cinder, Endgame / BLEndgame.
 */

#pragma once

#include "core_3x5.h"
#include "../shared/addons/thumb_outer.h"

/* core thumbs (inner 2; Variant A) + outer 3rd thumb */
#define POS_LH0 31
#define POS_LH1 32
#define POS_RH1 33
#define POS_RH0 34
#define POS_LHX 30   /* left  outer (3rd) thumb  */
#define POS_RHX 35   /* right outer (3rd) thumb  */

/* --- Layout adapter: 3x5 alpha (shared) + 3rd thumb, 2 core thumbs/hand, 3rd thumb --- */
#define KEYMAP_LAYER(L) \
    WEAVE_ALPHA_3x5(L) \
    THUMB_O_##L##_L CORE_##L##_thumb_L CORE_##L##_thumb_R THUMB_O_##L##_R
