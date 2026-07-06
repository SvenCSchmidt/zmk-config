/*
 * geom_3x5_2.h — shared geometry adapter for any board whose physical matrix is a
 * pure 3x5 + 2-thumbs/side core numbered CONTIGUOUSLY 0..33 (row-major; thumbs
 * 30/31 left, 32/33 right). By far the most common split/unibody layout.
 *
 *    0  1  2  3  4 |  5  6  7  8  9        row 0 (top)
 *   10 11 12 13 14 | 15 16 17 18 19        row 1 (home)
 *   20 21 22 23 24 | 25 26 27 28 29        row 2 (bottom)
 *           30 31  | 32 33                 thumbs
 *
 * The 30 alpha positions come from the shared core_3x5.h; this file only adds the
 * 2 thumbs/hand. Boards using it: re-gret, delta-omega, sweep.
 */

#pragma once

#include "core_3x5.h"

/* thumbs */
#define POS_LH0 30
#define POS_LH1 31
#define POS_RH1 32
#define POS_RH0 33

/* --- Layout adapter: the shared 34-key core (alpha + inner 2 thumbs/hand) --- */
#define KEYMAP_LAYER(L) WEAVE_CORE(L)
