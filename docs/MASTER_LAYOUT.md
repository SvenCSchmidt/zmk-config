# The master layout — how this repo works

> 🇩🇪 **Dieses Dokument gibt es auch auf Deutsch: [`MASTER_LAYOUT.de.md`](MASTER_LAYOUT.de.md).**

> This is the conceptual overview. For the **complete reference** — every shared
> variable documented and a full step-by-step board example — see
> [`REFERENCE.en.md`](REFERENCE.en.md) (English) / [`REFERENCE.de.md`](REFERENCE.de.md)
> (Deutsch).

This repo compiles **one** keymap for **many** keyboards. All shared content lives
once in `config/shared/`; each board only carries a thin *geometry adapter*
(`config/geometry/geom_<board>.h`) that weaves that content onto its physical keys
at compile time.

> TL;DR mental model: the shared **core** (3x5 + 2 thumbs) is defined once as small
> per-layer/row/hand blocks in `config/shared/core_blocks.dtsi`. Reusable **add-ons**
> (`config/shared/addons/*.h`) hold extra structures that several boards share (the
> outer pinky column, the 3rd thumb). Each `geom_<board>.h` picks which of these a
> board physically has and weaves them in its wiring order; anything a board doesn't
> share it writes by hand. Editing shared content changes every affected board at once.

### Change once → edit exactly one file

The whole point: a change that affects many boards touches **one** file.

| What you change | The one file to edit | Reaches |
|---|---|---|
| A core key / HomeRowMod / core thumb | `config/shared/core_blocks.dtsi` | every board |
| A combo | `config/shared/combos.dtsi` | every board |
| A macro | `config/shared/macros.dtsi` | every board |
| A behavior (HRM timing, Cadet Shift) | `config/shared/behaviors.dtsi` | every board |
| The outer pinky column (Esc/Tab/…) | `config/shared/addons/outer_col.h` | Corne, Cornholius, Splaytoraid |
| The 3rd (outer) thumb | `config/shared/addons/thumb_outer.h` | every board that has one |
| Common BLE settings | `config/shared_ble.conf` | every wireless board |
| Common USB settings | `config/shared_usb.conf` | every wired RP2040 board |

A `geom_<board>.h` contains **only** that board's physical position numbers (its
wiring) and its weave order — you never touch it for a content change, only when a
board is added or rewired. So no shared content is ever duplicated across boards.

---

## 1. The shared core (`core_blocks.dtsi`)

`config/shared/core_blocks.dtsi` is the single source of truth for the part *every*
board has in common: the **3×5 alpha block + inner 2 thumbs per hand** (34 keys),
holding the canonical Colemak-DH content. It is **not** a big grid — it is a set of
small, composable fragments, one per layer × row × hand, each expanding to a
space-separated list of ZMK bindings (no commas, so each drops straight into a
`bindings = <...>` array):

```
 CORE_<layer>_top_L    CORE_<layer>_top_R      row 0 (top alpha)     Q W F P B | J L U Y ;
 CORE_<layer>_home_L   CORE_<layer>_home_R     row 1 (home + HRMs)   A R S T G | M N E I O
 CORE_<layer>_bot_L    CORE_<layer>_bot_R      row 2 (bottom alpha)  Z X C D V | K H , . /
 CORE_<layer>_thumb_L  CORE_<layer>_thumb_R    inner 2 thumbs/hand
```

`<layer> ∈ { base, nav, num, fun, pad }`. Two weave helpers assemble the fragments in
canonical reading order:

- **`WEAVE_ALPHA_3x5(L)`** — the 30-key alpha block only (top, home, bottom; left hand
  then right hand per row), **no thumbs**. Boards weave their own thumb arrangement
  after it.
- **`WEAVE_CORE(L)`** — `WEAVE_ALPHA_3x5(L)` **plus** the inner 2 thumbs/hand = the full
  34-key `3×5+2` core.

There is **no** reserved outer grid, function row, or number row baked into the core.
Anything beyond the 34 keys is either a reusable **add-on** (§2) that a board opts into,
or written by hand in that board's adapter (§3). A board that physically has an extra
key it hasn't assigned yet weaves the filler **`RSVD_<layer>`** there (`&none` on base,
`&trans` on higher layers), so the key is inert but wired.

---

## 2. Reusable add-ons (`config/shared/addons/*.h`)

Structures that several — but not all — boards share live once as add-ons. Each add-on
defines fragments the same shape as the core's (per layer × position), and **every
fragment is `#ifndef`-guarded** so a board can override any single position by
`#define`-ing it *before* the include.

| Add-on | Fragments | Default content (base) | Used by |
|---|---|---|---|
| `addons/outer_col.h` | `OUTER_<layer>_<row>_<hand>` (top/home/bot × L/R) | L: `Esc` / `-` / Cadet-Shift `(` — R: `'` / `\` / Cadet-Shift `)` | Corne, Cornholius, Splaytoraid |
| `addons/thumb_outer.h` | `THUMB_O_<layer>_<hand>` (the 3rd/outer thumb) | unused (`&none`) | every board with a 3rd thumb |

**Override convention** — board overrides → add-on → core. Example, Cornholius' outer
column differs from the default in only three base positions:

```c
#define OUTER_base_home_L  &kp TAB    /* default was  -  */
#define OUTER_base_top_R   &kp BSPC   /* default was  '  */
#define OUTER_base_home_R  &kp RET    /* default was  \  */
#include "../shared/addons/outer_col.h"   /* fills the other positions */
```

Because the guards make the board's `#define` win, editing `outer_col.h` still reaches
every *non-overridden* position on every board that includes it. Cadet-Shift `(`/`)`
(hold = Shift, tap = bracket) needs the `cadet` hold-tap from `shared/behaviors.dtsi`.

---

## 3. The per-board geometry adapter (`geom_<board>.h`)

Each board has one adapter header with two artifacts, both derived from that board's
`matrix_transform` order:

1. **`POS_*`** symbols mapping each logical core key to that board's physical position
   number (`POS_LT0..POS_RB0`, `POS_LH0/LH1`, `POS_RH0/RH1`, plus any extras like
   `POS_LHX`/`POS_CENTER`). `combos.dtsi` and `behaviors.dtsi` reference only these
   symbols — **no raw position number ever appears in `shared/`.**
2. **`KEYMAP_LAYER(L)`** — weaves the core fragments (and any add-ons / hand-written
   edges) into the board's physical order. The thin keymap then just writes
   `bindings = <KEYMAP_LAYER(base)>` per layer.

### Shared adapters for the common shapes

Most boards are one of two contiguous shapes and don't need a bespoke adapter — they
`#include` a shared geometry header:

- **`geometry/geom_3x5_2.h`** — pure `3×5 + 2 thumbs/hand`, numbered `0..33`
  (`KEYMAP_LAYER(L) = WEAVE_CORE(L)`). Boards: cb34s, Urchin, re-gret, delta-omega,
  sweep. Its alpha `POS_*` come from `geometry/core_3x5.h` (positions `0..29`), shared
  so those 30 numbers are defined once.
- **`geometry/geom_3x5_3.h`** — `3×5 + 3 thumbs/hand`, numbered `0..35` (adds the outer
  thumb via `thumb_outer.h`; inner two are the core — "Variant A"). Boards: MNHTTN,
  Onyx Cinder, Endgame/BLEndgame.

### Bespoke adapters (supersets & odd matrices)

A board writes its own `KEYMAP_LAYER` when its matrix isn't one of those shapes — it
weaves the core fragments plus whatever it has:

- **TOTEM** (`geom_totem.h`) — a `3×5` core plus, per hand, one extra pinky on the
  bottom row and a 3rd (outer) thumb. Both use the shared add-ons: the extra pinky is
  the outer column's bottom key (`OUTER_*_bot`, Cadet-Shift by default) and the 3rd
  thumb is `THUMB_O_*` (unused by default).
- **Corne / Cornholius** (`geom_corne.h`, `geom_cornholius.h`) — outer pinky column
  (`OUTER_*`) around the core on all three rows, plus the 3rd thumb (`THUMB_O_*`);
  Cornholius adds a hand-written 4th row of outer modifiers (`C_R3_*`).
- **Le Chiffre** (`geom_chiffre.h`) — `3×5` with a **center key** (`CHIFF_CENTER`, the
  encoder push = Mute) spliced into the middle of the bottom row, shifting the
  bottom-right core to `26..30` — which is why it can't reuse `core_3x5.h`.
- **Splaytoraid** (`geom_splaytoraid.h` / `geom_splaytoraid36.h`) — core + a center
  encoder-push key + 3 thumbs/hand, with (40) or without (36) the outer column.

### Why the `KEYMAP_LAYER` name exists (argument prescan)

`KEYMAP_LAYER(L)` is a plain object-like weave, but keeping the indirection matters:
the C preprocessor pastes `##` tokens and expands fragment macros in a defined order,
so `KEYMAP_LAYER(base)` expands each `CORE_base_*` / `OUTER_base_*` fragment to its
bindings *before* the devicetree sees the `<...>` array. Each expanded fragment is
space-separated (never comma-separated), so the whole layer becomes one flat binding
list regardless of how many fragments/add-ons a board stitched together.

---

## 4. Editing content — the one-file rule

- **Change a core key / HRM / core thumb:** edit `config/shared/core_blocks.dtsi`.
  Never edit a thin `config/<board>.keymap`.
- **Change the outer column or 3rd thumb** for everyone that has it: edit the add-on
  (`addons/outer_col.h` / `addons/thumb_outer.h`). To change it for *one* board only,
  `#define` that position in the board's `geom_<board>.h` before the include.
- **`&none` vs `&trans`:** unused edges use `RSVD_<layer>` — `&none` on base (the key
  does nothing), `&trans` on higher layers (a populated base key falls through instead
  of dying). Add-ons follow the same rule for their higher layers.
- **Layer-access keys** (`&lt`/`&mo`/`&tog`) live on core positions every board has
  (the inner thumbs / core rows), so every board can reach every layer.

### Combos & HomeRowMods

`config/shared/combos.dtsi` and `config/shared/behaviors.dtsi` reference only `POS_*`
symbols, so they are board-agnostic. A combo that touches an **optional** (non-core)
position must be guarded so boards without that key are unaffected, e.g.:

```c
#ifdef HAS_OUTER_COL
   Cmb_outer { key-positions = <POS_LOB ...>; ... };
#endif
```

(No such guard is needed today — every current combo lives in the core.)

---

## 5. Board hardware: modules vs. vendored, and deleting old repos

A board's hardware definition (shield: `matrix_transform`, kscan, overlays) can come
from two places:

- **Upstream ZMK module** (preferred) — referenced in `config/west.yml`; CI fetches
  the real files at build time. Example: **Urchin** uses `duckyb/urchin-zmk-module`.
- **Vendored** under `boards/shields/<board>/` — a local copy, used when no suitable
  upstream module exists or when the module can't build against our ZMK revision.

Current sourcing:

| Board  | Source                                   | Why |
|--------|------------------------------------------|-----|
| Urchin | module `duckyb/urchin-zmk-module`        | upstream module, builds on `zmk main` |
| TOTEM  | vendored `boards/shields/totem/`         | the upstream module `BildermanKawasaki/zmk-keyboard-TOTEM` is pinned to ZMK v0.3 and not yet Zephyr-4.1/`main` compatible; the vendored copy builds on `main` and we control it |
| cb34s  | vendored `boards/shields/cb34s/`         | no distributable ZMK module exists — the upstream `bubbleology/zmk-config-cb34s` is itself a config repo that vendors the shield locally (and builds against the `AakashDabas/zmk@animated_widgets` fork); `bubbleology/CB34S` holds only the PCB/KiCad files |

> **Deleting your old per-board config repos is safe.** Nothing in this repo
> references `zmk-config-totem` / `zmk-config-cb34s` — the shield definitions are
> vendored and self-contained, and content lives in `shared/`. Switching TOTEM to the
> upstream module is a future option once that module supports Zephyr 4.1.

---

## 6. Config (`.conf`)

Common settings live once in a **shared conf**, picked per board by radio type via
`-DEXTRA_CONF_FILE` in `build.yaml`:

- **`config/shared_ble.conf`** — wireless/BLE boards (nRF: nice!nano & clones, xiao_ble).
  Enables Bluetooth, deep sleep, split battery reporting, debounce and combo limits.
- **`config/shared_usb.conf`** — wired USB-only boards (RP2040 "pro micro" clones such
  as Helios/SparkFun, e.g. Splaytoraid40). Same debounce/combo limits **without** any
  Bluetooth (those boards have no radio, so `CONFIG_BT` would fail).

A per-board **`config/<board>.conf`** exists **only** when a board needs something
specific (e.g. `cb34s.conf` carries the nice!view display widgets); it is merged on
top of the shared conf. Encoder EC11 config lives in the shield's own `<shield>.conf`.
`settings_reset` builds deliberately omit the shared conf.

---

## 7. Verification

Correctness is proven by preprocessing, not just "it builds":

- **`scratchpad/verify.py`** runs the C preprocessor over the *original*
  `zmk-config-totem` keymap and our `totem.keymap`, then compares every layer's
  expanded `bindings` and every combo. TOTEM's **34-key core must stay byte-identical**
  — the regression gate that proves a change to shared content didn't alter the
  canonical layout: 5/5 layers identical, all 37 combos identical. (TOTEM's four outer
  keys — its extra pinkies + 3rd thumbs — are now shared-add-on-driven, so they are
  excluded from the byte comparison.)
- **Per-board binding count** (e.g. `scratchpad/verify_cb34s.py`,
  `verify_endgame.py`): a board's keymap must preprocess to exactly the number of
  bindings its matrix has — `34` for a `3×5+2` core, `36` for a `3×5+3` board, etc.
  This catches a mis-woven `KEYMAP_LAYER` (wrong fragment order, a dropped or doubled
  position) before CI. The scripts reuse the sanitized tree `verify.py` builds.
- **CI** (GitHub Actions, `build.yaml`) then builds every board + a `settings_reset`
  image against `zmk main`.

Adding a board is described in [`ADDING_A_BOARD.md`](../ADDING_A_BOARD.md): pick the
matching `geom_3x5_*` header (or write a small `geom_<board>.h` weave for an odd
matrix), add a thin keymap and a `build.yaml` entry — no shared content changes.
