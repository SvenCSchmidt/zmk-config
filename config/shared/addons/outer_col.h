/*
 * addons/outer_col.h — reusable 3x6 OUTER pinky column (one extra column per hand,
 * on the top/home/bottom alpha rows). A board with a 3x6 outer column includes this
 * and weaves OUTER_<layer>_<row>_<hand> at its outer positions.
 *
 * Content (base): the canonical outer column —
 *        left            right
 *   top  Esc             '  (single quote)
 *   home -  (dash)       \  (backslash)
 *   bot  Cadet-Shift (   Cadet-Shift )     (Shift on hold, bracket on tap)
 *
 * Higher layers fall through (&trans) so the base outer keys stay reachable.
 *
 * OVERRIDE: every fragment is #ifndef-guarded, so a board can override any single
 * position by #defining it BEFORE including this file (the board's value wins,
 * this file fills the rest). Include order convention: board overrides -> this
 * add-on -> core_blocks.
 *
 * Requires the `cadet` hold-tap from shared/behaviors.dtsi.
 */

#pragma once

/* clang-format off */

/* ------------------------------- base ------------------------------------- */
#ifndef OUTER_base_top_L
#define OUTER_base_top_L   &kp ESC
#endif
#ifndef OUTER_base_home_L
#define OUTER_base_home_L  &kp MINUS
#endif
#ifndef OUTER_base_bot_L
#define OUTER_base_bot_L   &cadet LSHFT LPAR
#endif
#ifndef OUTER_base_top_R
#define OUTER_base_top_R   &kp SQT
#endif
#ifndef OUTER_base_home_R
#define OUTER_base_home_R  &kp BSLH
#endif
#ifndef OUTER_base_bot_R
#define OUTER_base_bot_R   &cadet RSHFT RPAR
#endif

/* -------------- higher layers: transparent (fall through) ----------------- */
#ifndef OUTER_nav_top_L
#define OUTER_nav_top_L    &trans
#endif
#ifndef OUTER_nav_home_L
#define OUTER_nav_home_L   &trans
#endif
#ifndef OUTER_nav_bot_L
#define OUTER_nav_bot_L    &trans
#endif
#ifndef OUTER_nav_top_R
#define OUTER_nav_top_R    &trans
#endif
#ifndef OUTER_nav_home_R
#define OUTER_nav_home_R   &trans
#endif
#ifndef OUTER_nav_bot_R
#define OUTER_nav_bot_R    &trans
#endif

#ifndef OUTER_num_top_L
#define OUTER_num_top_L    &trans
#endif
#ifndef OUTER_num_home_L
#define OUTER_num_home_L   &trans
#endif
#ifndef OUTER_num_bot_L
#define OUTER_num_bot_L    &trans
#endif
#ifndef OUTER_num_top_R
#define OUTER_num_top_R    &trans
#endif
#ifndef OUTER_num_home_R
#define OUTER_num_home_R   &trans
#endif
#ifndef OUTER_num_bot_R
#define OUTER_num_bot_R    &trans
#endif

#ifndef OUTER_fun_top_L
#define OUTER_fun_top_L    &trans
#endif
#ifndef OUTER_fun_home_L
#define OUTER_fun_home_L   &trans
#endif
#ifndef OUTER_fun_bot_L
#define OUTER_fun_bot_L    &trans
#endif
#ifndef OUTER_fun_top_R
#define OUTER_fun_top_R    &trans
#endif
#ifndef OUTER_fun_home_R
#define OUTER_fun_home_R   &trans
#endif
#ifndef OUTER_fun_bot_R
#define OUTER_fun_bot_R    &trans
#endif

#ifndef OUTER_pad_top_L
#define OUTER_pad_top_L    &trans
#endif
#ifndef OUTER_pad_home_L
#define OUTER_pad_home_L   &trans
#endif
#ifndef OUTER_pad_bot_L
#define OUTER_pad_bot_L    &trans
#endif
#ifndef OUTER_pad_top_R
#define OUTER_pad_top_R    &trans
#endif
#ifndef OUTER_pad_home_R
#define OUTER_pad_home_R   &trans
#endif
#ifndef OUTER_pad_bot_R
#define OUTER_pad_bot_R    &trans
#endif

/* clang-format on */
