/*
 * addons/thumb_outer.h — the 3rd (outer) thumb per hand, shared across every board
 * that physically has one (Corne, Cornholius, Splaytoraid, ...). One fragment per
 * hand per layer: THUMB_O_<layer>_L / THUMB_O_<layer>_R.
 *
 * Default: unused — &none on base, &trans on the higher layers. Assign real content
 * HERE to reach every board that includes this file, or override a single position
 * in a board by #defining it BEFORE the include (guarded -> the board's value wins).
 */

#pragma once

/* clang-format off */

#ifndef THUMB_O_base_L
#define THUMB_O_base_L  &none
#endif
#ifndef THUMB_O_base_R
#define THUMB_O_base_R  &none
#endif

#ifndef THUMB_O_nav_L
#define THUMB_O_nav_L   &trans
#endif
#ifndef THUMB_O_nav_R
#define THUMB_O_nav_R   &trans
#endif

#ifndef THUMB_O_num_L
#define THUMB_O_num_L   &trans
#endif
#ifndef THUMB_O_num_R
#define THUMB_O_num_R   &trans
#endif

#ifndef THUMB_O_fun_L
#define THUMB_O_fun_L   &trans
#endif
#ifndef THUMB_O_fun_R
#define THUMB_O_fun_R   &trans
#endif

#ifndef THUMB_O_pad_L
#define THUMB_O_pad_L   &trans
#endif
#ifndef THUMB_O_pad_R
#define THUMB_O_pad_R   &trans
#endif

/* clang-format on */
