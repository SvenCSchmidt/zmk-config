<!--
  This document is also available in German: docs/REFERENCE.de.md
-->
# Complete reference & board-authoring guide (English)

> 🇩🇪 **Diese Dokumentation gibt es auch auf Deutsch: [`REFERENCE.de.md`](REFERENCE.de.md).**

This is the exhaustive reference for the consolidated ZMK config: the full
architecture, **every** shared variable/macro documented, a **step-by-step guide for
adding a new board** with a complete worked example (a fictional board larger than
`3×5+2`, including an add-on override and an encoder), and the verification workflow.

For the shorter conceptual overview see [`MASTER_LAYOUT.md`](MASTER_LAYOUT.md); for the
quick checklist see [`ADDING_A_BOARD.md`](../ADDING_A_BOARD.md). This file is the
authoritative, complete version.

---

## Table of contents

1. [Mental model](#1-mental-model)
2. [Repository / file map](#2-repository--file-map)
3. [Coordinate system: the logical core](#3-coordinate-system-the-logical-core)
4. [Variable & macro catalog](#4-variable--macro-catalog)
   - [4.1 Position symbols `POS_*`](#41-position-symbols-pos_)
   - [4.2 Core content fragments `CORE_*`](#42-core-content-fragments-core_)
   - [4.3 Weave helpers `WEAVE_*` and `RSVD_*`](#43-weave-helpers-weave_-and-rsvd_)
   - [4.4 The `KEYMAP_LAYER` weave macro](#44-the-keymap_layer-weave-macro)
   - [4.5 Add-on: outer pinky column `OUTER_*`](#45-add-on-outer-pinky-column-outer_)
   - [4.6 Add-on: 3rd thumb `THUMB_O_*`](#46-add-on-3rd-thumb-thumb_o_)
   - [4.7 Board-local weave variables](#47-board-local-weave-variables)
   - [4.8 Behaviors (HomeRowMods + Cadet Shift)](#48-behaviors-homerowmods--cadet-shift)
   - [4.9 Macros `MCR_*`](#49-macros-mcr_)
   - [4.10 German aliases `DE_*`](#410-german-aliases-de_)
   - [4.11 Combos](#411-combos)
   - [4.12 Global settings](#412-global-settings)
   - [4.13 Kconfig (`.conf`) variables](#413-kconfig-conf-variables)
   - [4.14 Encoders (`sensor-bindings`)](#414-encoders-sensor-bindings)
5. [The five layers](#5-the-five-layers)
6. [Step-by-step: adding a board (full worked example)](#6-step-by-step-adding-a-board-full-worked-example)
7. [Verification](#7-verification)
8. [Board hardware sourcing](#8-board-hardware-sourcing)

---

## 1. Mental model

One keymap, many keyboards. All content is defined **once** in `config/shared/`; each
board carries only a thin *geometry adapter* that weaves that content onto its physical
keys at compile time (pure C-preprocessor, no build step).

- The **shared core** — a `3×5` alpha block + inner 2 thumbs per hand (34 keys) — is
  the part every keyboard has. It lives in `config/shared/core_blocks.dtsi` as small
  composable fragments.
- **Reusable add-ons** (`config/shared/addons/*.h`) hold structures that *several*
  boards share (the outer pinky column, the 3rd thumb). A board opts into the ones it
  physically has, and may **override** any single key.
- Each board's **`config/geometry/geom_<board>.h`** defines its physical position
  numbers and a `KEYMAP_LAYER(L)` macro that stitches the core (+ add-ons + any
  hand-written edges) into that board's matrix order.
- Each board's **`config/<board>.keymap`** is thin: include the geometry header + the
  shared files, then write `bindings = <KEYMAP_LAYER(base)>` per layer.

**Change once → rebuild everywhere.** Edit a core key in `core_blocks.dtsi` and every
board picks it up; edit an add-on and every board that includes it picks it up.

---

## 2. Repository / file map

```
zmk-config/
├── config/
│   ├── west.yml                     # unified ZMK revision + shield/widget modules
│   ├── shared_ble.conf              # common Kconfig for wireless (BLE) boards
│   ├── shared_usb.conf              # common Kconfig for wired USB-only boards
│   ├── <board>.keymap               # thin per-board keymap (one per board)
│   ├── <board>.conf                 # OPTIONAL per-board Kconfig extras (e.g. display)
│   ├── shared/
│   │   ├── core_blocks.dtsi         # THE shared 3x5+2 core content (source of truth)
│   │   ├── behaviors.dtsi           # HomeRowMods + Cadet-Shift hold-taps
│   │   ├── macros.dtsi              # MCR_* macros
│   │   ├── combos.dtsi              # all combos (symbolic POS_* positions)
│   │   ├── settings.dtsi            # global behavior settings (caps_word list)
│   │   ├── keys_de.h                # German-layout aliases (DE_*)
│   │   └── addons/
│   │       ├── outer_col.h          # reusable outer pinky column (OUTER_*)
│   │       └── thumb_outer.h        # reusable 3rd/outer thumb (THUMB_O_*)
│   └── geometry/
│       ├── core_3x5.h               # shared alpha POS_* (0..29) for contiguous boards
│       ├── geom_3x5_2.h             # shared adapter: pure 3x5 + 2 thumbs (0..33)
│       ├── geom_3x5_3.h             # shared adapter: 3x5 + 3 thumbs (0..35)
│       └── geom_<board>.h           # per-board bespoke adapters (supersets, odd matrices)
├── boards/
│   ├── shields/<name>/              # vendored split/shield hardware definitions
│   └── <vendor>/<board>/            # vendored HWv2 controller boards (nRF/STM32)
├── build.yaml                       # CI build matrix (one entry per board half)
├── zephyr/module.yml                # board_root: . (exposes boards/ for vendoring)
├── README.md
├── ADDING_A_BOARD.md                # quick checklist
└── docs/
    ├── MASTER_LAYOUT.md             # conceptual overview (EN)
    ├── REFERENCE.en.md              # THIS FILE — complete reference (EN)
    └── REFERENCE.de.md              # complete reference (DE)
```

Every board's thin keymap includes the shared files in this order:

```c
#include <behaviors.dtsi>              // ZMK system behaviors (&kp, &lt, &mt, ...)
#include <dt-bindings/zmk/keys.h>      // key codes (Q, SPACE, C_MUTE, ...)
#include <dt-bindings/zmk/bt.h>        // BT_SEL, BT_CLR, ...
#include <dt-bindings/zmk/outputs.h>   // OUT_USB, OUT_BLE, ...

#include "geometry/geom_<board>.h"     // POS_* + KEYMAP_LAYER for this board

#include "shared/keys_de.h"            // DE_* aliases
#include "shared/settings.dtsi"        // caps_word continue-list
#include "shared/behaviors.dtsi"       // HomeRowMods + cadet
#include "shared/macros.dtsi"          // MCR_*
#include "shared/combos.dtsi"          // combos (need POS_* from the geom header)
#include "shared/core_blocks.dtsi"     // CORE_* fragments + WEAVE_*/RSVD_*
```

> Order matters: the geometry header must precede `combos.dtsi`/`behaviors.dtsi` (they
> reference its `POS_*`), and a board that **overrides** an add-on key must `#define`
> the override inside its geom header *before* it includes the add-on.

---

## 3. Coordinate system: the logical core

Every board maps its physical keys onto the same **logical core** names. Read them as
`POS_<hand><row><finger-distance-from-center>`:

- **hand**: `L` (left) or `R` (right)
- **row**: `T` (top), `M` (middle/home), `B` (bottom)
- **index 0..4**: distance from the hand's inner edge outwards — `0` = inner (index
  finger's inner column), `4` = pinky column.

```
 LT0 LT1 LT2 LT3 LT4 | RT4 RT3 RT2 RT1 RT0     top row
 LM0 LM1 LM2 LM3 LM4 | RM4 RM3 RM2 RM1 RM0     home row
 LB0 LB1 LB2 LB3 LB4 | RB4 RB3 RB2 RB1 RB0     bottom row
             LH0 LH1 | RH1 RH0                 inner 2 thumbs / hand
```

These 34 names are the contract between shared content and every board. `combos.dtsi`
and `behaviors.dtsi` reference **only** these symbols, so no raw physical position
number ever appears in `config/shared/`. A board's geom header binds each name to its
own physical number.

Boards that have more keys add **extra** position symbols (documented in §4.1).

---

## 4. Variable & macro catalog

This section lists and documents **every** shared symbol.

### 4.1 Position symbols `POS_*`

Defined in a board's geometry header (or the shared `core_3x5.h`). They are plain
integers = the board's physical key-position index (as ZMK numbers keys in its
`matrix_transform`).

**Core (all 34 mandatory — every board defines these):**

| Symbol | Meaning | Symbol | Meaning |
|---|---|---|---|
| `POS_LT0..LT4` | left top row, inner→pinky | `POS_RT4..RT0` | right top row, pinky→inner |
| `POS_LM0..LM4` | left home row, inner→pinky | `POS_RM4..RM0` | right home row, pinky→inner |
| `POS_LB0..LB4` | left bottom row, inner→pinky | `POS_RB4..RB0` | right bottom row, pinky→inner |
| `POS_LH0, POS_LH1` | left inner 2 thumbs | `POS_RH1, POS_RH0` | right inner 2 thumbs |

**Optional extras (only on boards that physically have them):**

| Symbol | Meaning | Appears on |
|---|---|---|
| `POS_LOT / POS_ROT` | outer pinky column, **t**op row (L/R) | Corne, Cornholius, Splaytoraid40 |
| `POS_LOH / POS_ROH` | outer pinky column, **h**ome row (L/R) | Corne, Cornholius, Splaytoraid40 |
| `POS_LOB / POS_ROB` | outer pinky column, **b**ottom row (L/R) | Corne, Cornholius |
| `POS_LHX / POS_RHX` | 3rd (outer) thumb (L/R) | every board with a 3rd thumb |
| `POS_LBX / POS_RBX` | bottom-row outer pinky on TOTEM (its naming) | TOTEM |
| `POS_CENTER` | a single center key (encoder push) | Le Chiffre, Splaytoraid |

> Naming note: TOTEM predates the `LOB/ROB` convention and calls its bottom-row outer
> pinkies `POS_LBX/POS_RBX`; both refer to the same kind of key. New boards should use
> `LOB/ROB`.

**Shared helper `core_3x5.h`** — any board whose alpha block is numbered contiguously
`0..29` (row-major) `#include`s this to get all 30 alpha `POS_*` in one line instead of
repeating them. The includer then adds its own thumb/extra positions. Used indirectly
by every board that goes through `geom_3x5_2.h` / `geom_3x5_3.h`.

### 4.2 Core content fragments `CORE_*`

Defined in `config/shared/core_blocks.dtsi`. Each is an object-like macro expanding to
a **space-separated** list of ZMK bindings (no commas). Naming:
`CORE_<layer>_<row>_<hand>`.

| Fragment (× 5 layers) | Expands to | Count |
|---|---|---|
| `CORE_<L>_top_L` / `CORE_<L>_top_R` | top-row alpha, left / right hand | 5 each |
| `CORE_<L>_home_L` / `CORE_<L>_home_R` | home-row alpha (with HomeRowMods on base) | 5 each |
| `CORE_<L>_bot_L` / `CORE_<L>_bot_R` | bottom-row alpha | 5 each |
| `CORE_<L>_thumb_L` / `CORE_<L>_thumb_R` | inner 2 thumbs, left / right | 2 each |

where `<L> ∈ { base, nav, num, fun, pad }` — so there are **8 fragments × 5 layers = 40**
`CORE_*` macros. Each row fragment yields 5 bindings, each thumb fragment 2, so one
layer's worth of core is `5·6 + 2·2 = 34` bindings.

The **content** of the base layer (the canonical Colemak-DH, DE host) is:

```
top     Q  W  F  P  B   |  J  L  U  Y  ;
home    A  R  S  T  G   |  M  N  E  I  O      (home keys are HomeRowMods, see §4.8)
bottom  Z  X  C  D  V   |  K  H  ,  .  /
thumbs  [C/S/A·Esc] [nav·Ret] | [num·Spc] [C/S/A·Del]
```

The other four layers (`nav`, `num`, `fun`, `pad`) use the same 40 fragment names with
different bindings; the exact bindings are in `core_blocks.dtsi` (the source of truth)
and summarised in §5. **To change any core key, edit that one fragment** — never a
board keymap.

### 4.3 Weave helpers `WEAVE_*` and `RSVD_*`

Also in `core_blocks.dtsi`:

| Macro | Expands to | Use |
|---|---|---|
| `WEAVE_ALPHA_3x5(L)` | the 30 alpha bindings (top,home,bot; L then R per row), **no thumbs** | boards that place a custom thumb row after the alphas |
| `WEAVE_CORE(L)` | `WEAVE_ALPHA_3x5(L)` + inner 2 thumbs/hand = 34 bindings | pure `3×5+2` boards (`KEYMAP_LAYER(L) = WEAVE_CORE(L)`) |
| `RSVD_base` | `&none` | a physically-present but unassigned key, base layer |
| `RSVD_nav/num/fun/pad` | `&trans` | same key on higher layers (falls through) |

`RSVD_<L>` is the "reserved/inert" filler: weave `RSVD_##L` at a position the board has
but whose function isn't decided yet — it does nothing on base and is transparent above,
but is wired and ready to be replaced with real content later.

### 4.4 The `KEYMAP_LAYER` weave macro

Every geometry header defines exactly one:

```c
#define KEYMAP_LAYER(L) <fragments, in this board's physical order>
```

The thin keymap calls it once per layer: `bindings = <KEYMAP_LAYER(base)>;` etc. The
preprocessor pastes the layer name into each `CORE_##L##_*` / `OUTER_##L##_*` /
`THUMB_O_##L##_*` fragment and expands them **before** devicetree sees the `<...>`
array. Because every fragment is space-separated, the result is one flat binding list
regardless of how many pieces were stitched together. The number of bindings it emits
**must equal the board's physical key count** (that is what §7 verifies).

### 4.5 Add-on: outer pinky column `OUTER_*`

`config/shared/addons/outer_col.h` — one extra column per hand on the three alpha rows
(6 keys). Fragment name: `OUTER_<layer>_<row>_<hand>` (`row ∈ {top,home,bot}`,
`hand ∈ {L,R}`) → **6 fragments × 5 layers = 30** `OUTER_*` macros.

Default content:

| | left | right |
|---|---|---|
| top (base) | `&kp ESC` | `&kp SQT` (`'`) |
| home (base) | `&kp MINUS` (`-`) | `&kp BSLH` (`\`) |
| bottom (base) | `&cadet LSHFT LPAR` | `&cadet RSHFT RPAR` |
| all higher layers | `&trans` | `&trans` |

`&cadet …` is Space-Cadet Shift (§4.8): hold = shift, tap = a bracket `(`/`)`.

**Every fragment is `#ifndef`-guarded**, so a board overrides a single position by
`#define`-ing it *before* `#include`-ing the add-on:

```c
#define OUTER_base_top_L &kp TAB   /* this board: Tab instead of the default Esc */
#include "../shared/addons/outer_col.h"   /* fills the other 29 fragments */
```

Requires the `cadet` behavior from `behaviors.dtsi` (already included by every keymap).

### 4.6 Add-on: 3rd thumb `THUMB_O_*`

`config/shared/addons/thumb_outer.h` — the 3rd (outer) thumb per hand. Fragment name:
`THUMB_O_<layer>_<hand>` (`hand ∈ {L,R}`) → **2 fragments × 5 layers = 10** macros.

Default content: **unused** — `&none` on base, `&trans` on the higher layers. Same
`#ifndef` override mechanism as `OUTER_*`: to give this thumb real content on *one*
board, `#define THUMB_O_base_L …` before the include; to give it to *every* board that
has a 3rd thumb, edit `thumb_outer.h` directly.

### 4.7 Board-local weave variables

Some boards have keys that no other board shares; those live in the board's own geom
header (not in `shared/`). They follow the `<NAME>_<layer>` convention so
`KEYMAP_LAYER` can paste the layer in. Current examples:

| Variable | Board | Meaning |
|---|---|---|
| `CHIFF_CENTER_<layer>` | Le Chiffre | the center key (`&kp C_MUTE` on base, `&trans` above) |
| `SPLY_CENTER_<layer>` | Splaytoraid 40/36 | center encoder-push key (`&kp C_MUTE` / `&trans`) |
| `C_R3_<layer>_L/R` | Cornholius | the 4th-row outer modifier triplets (Ctrl/Win/Alt · AltGr/Win/Ctrl) |
| `C_R3_TRANS` | Cornholius | shorthand `&trans &trans &trans` for that row on higher layers |

When you add a board with a unique key, create such a variable in its geom header,
following this naming so `KEYMAP_LAYER` can weave it per layer.

### 4.8 Behaviors (HomeRowMods + Cadet Shift)

`config/shared/behaviors.dtsi`. All are `zmk,behavior-hold-tap` with `#binding-cells =
<2>` (invoked `&<name> <hold-param> <tap-param>`). Parameters are 1:1 from the canonical
TOTEM keymap.

| Label | Node name | Hold / tap role | tapping-term-ms | flavor | positional? |
|---|---|---|---|---|---|
| `&hmg` | `homerow_mod_gui` | GUI (on hold) / letter | 500 | balanced | no |
| `&hmla` | `homerow_mod_lalt` | Left-Alt / letter | 400 | balanced | no |
| `&hmls` | `homerow_mod_lshift` | Left-Shift / letter | 150 | balanced | yes (right core) |
| `&hmlc` | `homerow_mod_lctrl` | Left-Ctrl / letter | 120 | balanced | yes (right core) |
| `&hmrc` | `homerow_mod_rctrl` | Right-Ctrl / letter | 120 | balanced | yes (left core) |
| `&hmrs` | `homerow_mod_rshift` | Right-Shift / letter | 180 | balanced | yes (left core) |
| `&hmra` | `homerow_mod_ralt` | Right-Alt / letter | 800 | tap-preferred | no |
| `&cadet` | `cadet_shift` | Shift / bracket | 200 | balanced | no |

Shared across all: `quick-tap-ms = <0>`. `hmlc`/`hmrc` also set `hold-while-undecided`.

**Positional hold-taps** (`hmlc/hmrc/hmls/hmrs`) restrict the hold to when a key on the
**opposite** hand's core is pressed, using `hold-trigger-key-positions`. Two shared
lists build those from `POS_*`:

- `TRIGGER_RIGHT` = the 15 right-hand core positions (`RT4..RT0 RM4..RM0 RB4..RB0`) —
  used by the **left** HRMs.
- `TRIGGER_LEFT` = the 15 left-hand core positions — used by the **right** HRMs.

Because these are built from symbols, they are board-independent. On base, the home row
is: `&hmg LGUI A` `&hmla LALT R` `&hmls LSHFT S` `&hmlc LCTRL T` `G` | `M` `&hmrc RCTRL
N` `&hmrs RSHFT E` `&hmla LALT I` `&hmg RGUI O`, plus `&hmra RALT X` and `&hmra RALT
DOT` on the bottom row.

### 4.9 Macros `MCR_*`

`config/shared/macros.dtsi`, all `zmk,behavior-macro` (`#binding-cells = <0>`, invoked
`&MCR_…`). Tuned for a **German host layout**.

| Macro | Produces | Sequence |
|---|---|---|
| `&MCR_DQT` | `"` then a space | press LSHFT · tap `'` · release LSHFT · tap Space |
| `&MCR_SCH` | `sch` (only the S capitalised) | tap S · release both shifts · tap C, H |
| `&MCR_SQT` | `'` then a space | tap `'` · tap Space |
| `&MCR_TILDE` | `~` then a space | press RSHFT · tap grave · release RSHFT · tap Space |
| `&MCR_DEGREE` | `°` | press RALT+LSHFT · tap `;` · release both |

### 4.10 German aliases `DE_*`

`config/shared/keys_de.h`. Convenience aliases for authoring; the canonical layers
produce these via combos rather than depending on the aliases. **Host must be set to a
German (DE) keyboard layout** — the umlauts come out as AltGr (`RA = RIGHT_ALT`)
combinations.

| Alias | Value | Character |
|---|---|---|
| `DE_AE` | `RA(Q)` | ä |
| `DE_OE` | `RA(P)` | ö |
| `DE_UE` | `RA(Y)` | ü |
| `DE_SS` | `RA(S)` | ß |
| `DE_EUR` | `RA(N5)` | € |

### 4.11 Combos

`config/shared/combos.dtsi` — 37 combos, all on core positions (so board-independent;
no guards needed yet). `layers = <0..4>` map to `base nav num fun pad`.

| Name | Output | Positions | Layers |
|---|---|---|---|
| `Cmb_UmlA` | ä (`RA(Q)`) | LT0 LM0 | base |
| `Cmb_UmlO` | ö (`RA(P)`) | RT0 RM0 | base |
| `Cmb_UmlU` | ü (`RA(Y)`) | RT2 RM2 | base |
| `Cmb_UmlS` | ß (`RA(S)`) | LT2 LM2 | base |
| `Cmb_EUR` | € (`RA(N5)`) | RM2 RB2 | base |
| `Cmb_ESC` | Esc | LT0 LT1 | all |
| `Cmb_TAB` | Tab | LB2 LB3 | base,nav,num,pad |
| `Cmb_CLCKs` | Caps Lock | LM0 LM1 LM2 LM3 | nav |
| `Cmb_CapsWord` | `&caps_word` | LM2 RM2 | base |
| `Cmb_BSPC` | Backspace | RB3 RB2 | base,num,pad |
| `Cmb_DEL` | Delete | RB2 RB1 | base,num,pad |
| `Cmb_CtrlBSPC` | Ctrl+Backspace | RB3 RB2 RB1 | base,num |
| `Cmb_CtrlDEL` | Ctrl+Delete | RB2 RB1 RB0 | base,num |
| `Cmb_WordLeft` | Ctrl+← | LM1 LM2 | nav |
| `Cmb_WordRight` | Ctrl+→ | LM2 LM3 | nav |
| `Cmb_Cut` | Ctrl+X | LB0 LB1 | base |
| `Cmb_Copy` | Ctrl+C | LB1 LB2 | base |
| `Cmb_Paste` | Ctrl+V | LB0 LB1 LB2 | base |
| `Cmb_Reset` | `&bootloader` | LM1 LM2 LM3 | pad |
| `Cmb_UNDO` | Ctrl+Z | LT2 LT3 | base |
| `Cmb_REDO` | Ctrl+Y | RT3 RT2 | base |
| `Cmb_ALL` | Ctrl+A | LM0 LM1 | base |
| `Cmb_LPAR` | `(` | LT4 LM4 | base |
| `Cmb_RPAR` | `)` | RT4 RM4 | base |
| `Cmb_LBKT` | `[` | LM4 LB4 | base |
| `Cmb_RBKT` | `]` | RM4 RB4 | base |
| `Cmb_BSLH` | `\` | RM0 RB0 | base |
| `Cmb_MINUS` | `-` | LT3 LM3 | base |
| `Cmb_EQUAL` | `=` | RT3 RM3 | base |
| `Cmb_UNDER` | `_` | LM3 LB3 | base |
| `Cmb_PLUS` | `+` | RM3 RB3 | base |
| `Cmb_COLON` | `:` | RT1 RM1 | base |
| `Cmb_PIPE` | `|` | RM1 RB1 | base |
| `Cmb_MCR_SCH` | `&MCR_SCH` (sch) | LM2 LB2 | base |
| `Cmb_MCR_DQT` | `&MCR_DQT` (") | LT1 LM1 | base |
| `Cmb_MCR_TILDE` | `&MCR_TILDE` (~) | LM1 LB1 | base,num |
| `Cmb_MCR_SQT` | `&MCR_SQT` (') | LM0 LB0 | base,num |

### 4.12 Global settings

`config/shared/settings.dtsi`:

```c
&caps_word { continue-list = <UNDERSCORE MINUS BSPC DEL>; };
```

Extends ZMK's Caps-Word so it stays active across `_`, `-`, Backspace and Delete.

### 4.13 Kconfig (`.conf`) variables

Chosen per board by radio type via `-DEXTRA_CONF_FILE=…` in `build.yaml`. A per-board
`config/<board>.conf` (if present) is merged on top for board-specific extras.

**`config/shared_ble.conf`** — wireless / BLE boards (nRF: nice!nano & clones, xiao_ble):

| Variable | Value | Meaning |
|---|---|---|
| `CONFIG_BT` | `y` | enable Bluetooth |
| `CONFIG_BT_CTLR_TX_PWR_PLUS_8` | `y` | +8 dBm TX power |
| `CONFIG_ZMK_SLEEP` | `y` | deep-sleep support |
| `CONFIG_ZMK_IDLE_SLEEP_TIMEOUT` | `600000` | sleep after 10 min idle (ms) |
| `CONFIG_BT_MAX_CONN` | `6` | max simultaneous connections |
| `CONFIG_BT_MAX_PAIRED` | `6` | max paired profiles |
| `CONFIG_ZMK_KSCAN_DEBOUNCE_PRESS_MS` | `1` | eager press debounce |
| `CONFIG_ZMK_KSCAN_DEBOUNCE_RELEASE_MS` | `5` | release debounce |
| `CONFIG_ZMK_COMBO_MAX_PRESSED_COMBOS` | `8` | combo engine limits… |
| `CONFIG_ZMK_COMBO_MAX_COMBOS_PER_KEY` | `16` | …raised because the shared config… |
| `CONFIG_ZMK_COMBO_MAX_KEYS_PER_COMBO` | `5` | …uses many overlapping combos |
| `CONFIG_ZMK_BLE_EXPERIMENTAL_FEATURES` | `y` | experimental BLE stack |
| `CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING` | `y` | peripheral battery report (split) |

**`config/shared_usb.conf`** — wired USB-only boards (RP2040 "pro micro" clones). Same
debounce + combo limits, **no** Bluetooth:

| Variable | Value | Meaning |
|---|---|---|
| `CONFIG_ZMK_USB` | `y` | USB HID output |
| `CONFIG_ZMK_KSCAN_DEBOUNCE_PRESS_MS` | `1` | eager press debounce |
| `CONFIG_ZMK_KSCAN_DEBOUNCE_RELEASE_MS` | `5` | release debounce |
| `CONFIG_ZMK_COMBO_MAX_PRESSED_COMBOS` | `8` | combo limits (as above) |
| `CONFIG_ZMK_COMBO_MAX_COMBOS_PER_KEY` | `16` | |
| `CONFIG_ZMK_COMBO_MAX_KEYS_PER_COMBO` | `5` | |

> Never put `CONFIG_BT…` in a USB board's config — those boards have no radio and the
> build fails. Encoder `CONFIG_EC11…` belongs in the **shield's** own `<shield>.conf`,
> not the shared conf. `settings_reset` builds deliberately omit the shared conf.

### 4.14 Encoders (`sensor-bindings`)

Rotary encoders are orthogonal to the key grid — they are **sensors**, not key
positions, so they never appear in `KEYMAP_LAYER`. A board with encoders adds a
`sensor-bindings` property to each layer node in its thin keymap, in the same order as
the shield's `sensors = <…>` list. The common behavior is `&inc_dec_kp <ccw> <cw>`.

Example (KLOTZ, two encoders — volume, cursor):

```c
#define ENCODERS  &inc_dec_kp C_VOL_DN C_VOL_UP   &inc_dec_kp LEFT RIGHT
...
base { bindings = <KEYMAP_LAYER(base)>; sensor-bindings = <ENCODERS>; };
```

The EC11 hardware is enabled in the shield's own `<shield>.conf`
(`CONFIG_EC11=y`, `CONFIG_EC11_TRIGGER_GLOBAL_THREAD=y`).

---

## 5. The five layers

Layer indices and how each is reached (all access keys sit on core positions every
board has, so every board can reach every layer):

| # | Name | Reached by | Purpose |
|---|---|---|---|
| 0 | `base` | default | Colemak-DH letters, HomeRowMods, thumb layer-taps |
| 1 | `nav` | hold left inner thumb (`&lt 1 RET`) | arrows, Home/End/PgUp/PgDn, BT profile select, media |
| 2 | `num` | hold right inner thumb (`&lt 2 SPACE`) | numbers + symbols (shifted number row, brackets) |
| 3 | `fun` | `&mo 3` (right thumb on `nav`) | F1–F12 + modifiers |
| 4 | `pad` | `&tog 4` (left thumb on `num`); `&to 0` returns | numpad; `&bootloader` combo lives here |

The exact bindings of all five layers are in `config/shared/core_blocks.dtsi` (the
source of truth). Summary of the non-base layers:

- **nav**: top = Esc/PrtScr/↑/Ins/PgUp · BT_SEL 0–4; home = Home/←/↓/→/End · app/mods;
  bottom = ScrLk/Pause/BSPC/Del/PgDn · BT prev/next/clear.
- **num**: top = shifted `1..5` / `6..0`; home = `1..5` / `6..0` with HRMs; bottom =
  `°`, `'`, BSPC, Del, `[` / `]`, `,`, `.`.
- **fun**: top = F1–F5 / F6–F10; home = GUI/Alt/Shift/Ctrl/F11 / F12/…; a `&tog 4` and
  `&mo 3` live on the thumbs.
- **pad**: right hand is a numpad (`7 8 9 - / * 4 5 6 + . 1 2 3 ,`); `&to 0` top-left
  returns to base.

---

## 6. Step-by-step: adding a board (full worked example)

We add a **fictional** board to show the whole flow, deliberately chosen to be *larger*
than `3×5+2` and to need an **override**.

### The fictional board: **"Kolibri"**

- Wireless (nice!nano, nRF52840).
- **3×6 + 3 thumbs/hand = 42 keys** (an outer pinky column on all three rows, plus a
  3rd/outer thumb per hand) — a superset, so it can't reuse `geom_3x5_2/3`.
- One rotary **encoder** on the left half → volume.
- Two **overrides** vs. the shared defaults:
  1. the outer column's top-left key should be **Tab**, not the default **Esc**;
  2. the 3rd thumbs should be **GUI** keys, not the default `&none`.

Physical matrix (contiguous `0..41`, row-major, as ZMK numbers it):

```
 0  1  2  3  4  5 |  6  7  8  9 10 11     row 0 (0,11 = outer pinky column)
12 13 14 15 16 17 | 18 19 20 21 22 23     row 1 (12,23 = outer pinky column)
24 25 26 27 28 29 | 30 31 32 33 34 35     row 2 (24,35 = outer pinky column)
      36 37 38    | 39 40 41              thumbs (36,41 = 3rd/outer thumb)
```

### Step 1 — provide the hardware definition

Vendor the shield under `boards/shields/kolibri/` (or reference an upstream module in
`config/west.yml` if a good one exists). Copy **only** the hardware files —
`kolibri.overlay` / `kolibri.dtsi` / any `*-layouts.dtsi`, `Kconfig.shield`,
`Kconfig.defconfig`, and for a split `kolibri.zmk.yml`. **Do not** copy the source
repo's `keymap`, `keys_*.h`, or radio `.conf` — those come from `config/shared/`.

`Kconfig.shield` must name the guard after the shield:

```kconfig
config SHIELD_KOLIBRI
    def_bool $(shields_list_contains,kolibri)
```

### Step 2 — write the geometry adapter `config/geometry/geom_kolibri.h`

Because Kolibri is a superset, it gets its own header. Note the **overrides are
`#define`d before the add-on includes**.

```c
/*
 * geom_kolibri.h — fictional 3x6 + 3-thumbs/hand board (42 keys), contiguous 0..41.
 * Outer pinky column on all 3 rows (addons/outer_col.h) + a 3rd thumb per hand
 * (addons/thumb_outer.h). Two overrides: Tab top-left, GUI on the 3rd thumbs.
 */
#pragma once

/* --- core positions (logical core -> Kolibri physical numbers) --- */
/* top    */ #define POS_LT0 1
             #define POS_LT1 2
             #define POS_LT2 3
             #define POS_LT3 4
             #define POS_LT4 5
             #define POS_RT4 6
             #define POS_RT3 7
             #define POS_RT2 8
             #define POS_RT1 9
             #define POS_RT0 10
/* home   */ #define POS_LM0 13
             #define POS_LM1 14
             #define POS_LM2 15
             #define POS_LM3 16
             #define POS_LM4 17
             #define POS_RM4 18
             #define POS_RM3 19
             #define POS_RM2 20
             #define POS_RM1 21
             #define POS_RM0 22
/* bottom */ #define POS_LB0 25
             #define POS_LB1 26
             #define POS_LB2 27
             #define POS_LB3 28
             #define POS_LB4 29
             #define POS_RB4 30
             #define POS_RB3 31
             #define POS_RB2 32
             #define POS_RB1 33
             #define POS_RB0 34
/* inner 2 thumbs/hand */
             #define POS_LH0 37
             #define POS_LH1 38
             #define POS_RH1 39
             #define POS_RH0 40

/* --- extras Kolibri physically has (symbols so combos/HRM could target them) --- */
#define POS_LOT 0    /* outer column, top,    left  */
#define POS_LOH 12   /* outer column, home,   left  */
#define POS_LOB 24   /* outer column, bottom, left  */
#define POS_ROT 11   /* outer column, top,    right */
#define POS_ROH 23   /* outer column, home,   right */
#define POS_ROB 35   /* outer column, bottom, right */
#define POS_LHX 36   /* 3rd (outer) thumb, left     */
#define POS_RHX 41   /* 3rd (outer) thumb, right    */

/* --- OVERRIDES: must come BEFORE the add-on includes (guards make ours win) --- */
#define OUTER_base_top_L  &kp TAB    /* Kolibri: Tab (default was Esc) */
#define THUMB_O_base_L    &kp LGUI   /* Kolibri: GUI on the 3rd thumbs (default &none) */
#define THUMB_O_base_R    &kp RGUI

#include "../shared/addons/outer_col.h"    /* fills the rest of the outer column */
#include "../shared/addons/thumb_outer.h"  /* fills the rest of the 3rd-thumb frags */

/* --- weave the core + add-ons into Kolibri's physical order --- */
#define KEYMAP_LAYER(L) \
    OUTER_##L##_top_L   CORE_##L##_top_L   CORE_##L##_top_R   OUTER_##L##_top_R  \
    OUTER_##L##_home_L  CORE_##L##_home_L  CORE_##L##_home_R  OUTER_##L##_home_R \
    OUTER_##L##_bot_L   CORE_##L##_bot_L   CORE_##L##_bot_R   OUTER_##L##_bot_R  \
    THUMB_O_##L##_L CORE_##L##_thumb_L CORE_##L##_thumb_R THUMB_O_##L##_R
```

Count the weave for one layer: `6 (outer) + 30 (alpha) + 4 (core thumbs) + 2 (3rd
thumbs) = 42` — matches the matrix. Good.

> Why the overrides sit before the includes: the add-on wraps every default in
> `#ifndef`. If you `#define OUTER_base_top_L` first, the add-on's `#ifndef` skips its
> own default and keeps yours. Define it *after* the include and it's a redefinition
> error. This one board's override does **not** affect any other board.

### Step 3 — write the thin keymap `config/kolibri.keymap`

```c
/*
 * Kolibri (fictional) — thin keymap. 3x6 + 3 thumbs/hand on nice_nano, one left
 * encoder (volume). All content lives in config/shared/.
 */
#include <behaviors.dtsi>
#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/bt.h>
#include <dt-bindings/zmk/outputs.h>

#include "geometry/geom_kolibri.h"

#include "shared/keys_de.h"
#include "shared/settings.dtsi"
#include "shared/behaviors.dtsi"
#include "shared/macros.dtsi"
#include "shared/combos.dtsi"
#include "shared/core_blocks.dtsi"

/* one encoder on the left half -> volume down/up */
#define ENCODERS  &inc_dec_kp C_VOL_DN C_VOL_UP

/ {
    keymap {
        compatible = "zmk,keymap";
        base { label = "base"; bindings = <KEYMAP_LAYER(base)>; sensor-bindings = <ENCODERS>; };
        nav  { label = "nav";  bindings = <KEYMAP_LAYER(nav)>;  sensor-bindings = <ENCODERS>; };
        num  { label = "num";  bindings = <KEYMAP_LAYER(num)>;  sensor-bindings = <ENCODERS>; };
        fun  { label = "fun";  bindings = <KEYMAP_LAYER(fun)>;  sensor-bindings = <ENCODERS>; };
        pad  { label = "pad";  bindings = <KEYMAP_LAYER(pad)>;  sensor-bindings = <ENCODERS>; };
    };
};
```

(If the board has no encoder, drop the `sensor-bindings` and the `ENCODERS` define.)

### Step 4 — add a per-board conf only if needed

Kolibri needs the encoder's EC11 — but that lives in the **shield's** own
`boards/shields/kolibri/kolibri.conf`:

```kconfig
CONFIG_EC11=y
CONFIG_EC11_TRIGGER_GLOBAL_THREAD=y
```

No `config/kolibri.conf` is needed unless the board has something extra (e.g. a
display). The radio settings come from `shared_ble.conf` in the next step.

### Step 5 — add to `build.yaml`

```yaml
  # --- Kolibri (fictional 3x6+3 BLE demo; vendored shield boards/shields/kolibri) ---
  - board: nice_nano//zmk
    shield: kolibri
    cmake-args: -DEXTRA_CONF_FILE=../../config/shared_ble.conf
```

For a **split** board you would add `kolibri_left` and `kolibri_right` entries; for a
**wired RP2040** board use `sparkfun_pro_micro_rp2040//zmk` + `shared_usb.conf`; add a
`settings_reset` entry for the controller if you want a reset image.

### Step 6 — verify (before pushing)

Copy `scratchpad/verify_endgame.py` to `verify_kolibri.py`, change the target keymap to
`kolibri.keymap` and the expected count to `42`, then run the TOTEM gate plus the new
check:

```
python3 verify.py            # TOTEM must stay byte-identical (shared content untouched)
python3 verify_kolibri.py    # kolibri must expand to exactly 42 bindings on all 5 layers
```

Then commit and push; CI (`build.yaml`) builds the board against `zmk main`.

### What you did **not** touch

No shared file changed. Adding Kolibri touched only: its vendored shield, its
`geom_kolibri.h`, its thin `kolibri.keymap`, and one `build.yaml` entry. Every other
board is unaffected — and if you later change a core key in `core_blocks.dtsi`, Kolibri
inherits it automatically.

---

## 7. Verification

Correctness is proven by preprocessing, not just "it builds":

- **`scratchpad/verify.py`** runs the C preprocessor over the *original*
  `zmk-config-totem` keymap and our `totem.keymap`, comparing every layer's expanded
  `bindings` and every combo. TOTEM must stay **byte-identical** — the regression gate
  that proves a shared-content edit didn't alter the canonical layout (5/5 layers, all
  37 combos identical).
- **Per-board binding count** (`verify_cb34s.py`, `verify_endgame.py`, …): a board's
  keymap must preprocess to exactly its physical key count (34 for `3×5+2`, 36 for
  `3×5+3`, 42 for the Kolibri example). This catches a mis-woven `KEYMAP_LAYER` (wrong
  order, dropped/doubled position) before CI. The scripts reuse the sanitized tree
  `verify.py` builds.
- **CI** (GitHub Actions, `build.yaml`) then builds every board + a `settings_reset`
  image against `zmk main`.

---

## 8. Board hardware sourcing

A board's hardware (shield `matrix_transform`, kscan, overlays; or a controller board)
comes from one of:

- **Upstream ZMK module** — referenced in `config/west.yml`; CI fetches it at build
  time. Preferred when a module builds cleanly against our `zmk main`. Current modules:
  Urchin (`duckyb/urchin-zmk-module`), re-gret (`rschenk/zmk-keyboard-re-gret` @ v0.4),
  forager (`carrefinho/forager-zmk-module`), delta-omega
  (`unspecworks/zmk-keyboard-delta-omega`), plus the `nice-view-gem` display and the
  `zmk-rgbled-widget` (a hard dependency of delta-omega, needed workspace-wide).
- **Vendored** under `boards/` — a local copy, when no distributable module exists or a
  module can't build against our revision. Shields live in `boards/shields/<name>/`;
  full HWv2 controller boards (migrated to Zephyr 4.1) live in
  `boards/<vendor>/<board>/` (Cornholius, Le Chiffre BLE, Le Chiffre 36 STM32).
  `zephyr/module.yml` sets `board_root: .` so `boards/` is on the search path.

Deleting your old per-board `zmk-config-*` repos is safe: the shield/board definitions
are vendored (or pulled as modules) and all content lives in `config/shared/`.
