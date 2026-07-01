# The master layout — how this repo works

This repo compiles **one** keymap for **many** keyboards. All layer content lives
once in `config/shared/`; each board only carries a thin *geometry adapter* that
places that content onto its physical keys at compile time. This document explains
the master layout in detail: the grid, the naming, how a board consumes it, how to
edit it, and how it is verified.

> TL;DR mental model: `master_layers.dtsi` is a big keyboard-shaped grid of key
> bindings. A board's `geom_<board>.h` is a stencil that picks the holes that board
> physically has and lays them out in that board's wiring order. Editing the grid
> changes every board at once.

---

## 1. The master grid

`config/shared/master_layers.dtsi` defines every layer as one `KM_<layer>` macro
over a fixed grid sized to the largest keyboard we expect to support:

**6 rows × 7 columns per hand + 3 thumbs per hand = 90 slots.**

```
 columns per hand (outer -> inner):  OUT  pinky ring mid index inner  INX
                                     └new   └────── core (5) ──────┘  └new
 rows (top -> bottom):
   R0  FN     function row     optional
   R1  NUM    number row       optional
   R2  TOP    top alpha        CORE    Q W F P B | J L U Y ;
   R3  HOME   home row         CORE    A R S T G | M N E I O   (with HomeRowMods)
   R4  BOT    bottom alpha     CORE    Z X C D V | K H , . /
   R5  EXT    extra bottom     optional
 thumbs per hand (outer -> inner):  TH_O (optional)   TH_M (core)   TH_I (core)
```

The **core** — the middle 5 columns × 3 rows plus the inner 2 thumbs per hand — is
the 34-key `3×5+2` layout that *every* board has, and it is the only part currently
filled with content (the canonical Colemak-DH layout from the TOTEM). Every other
slot is **reserved**: `&none` on `base`, `&trans` on higher layers, waiting to be
populated when a board that physically has those keys is added.

### Slot order (how `KM_<layer>` and `LAYOUT()` line up)

Slots are listed **row-major, both hands per row** (left columns outer→inner, then
right columns inner→outer), rows `FN`→`EXT`, then the 6 thumbs
(`L_TH_O, L_TH_M, L_TH_I, R_TH_I, R_TH_M, R_TH_O`). Index map:

| slots      | region            |
|------------|-------------------|
| `s00..s13` | FN row (14)       |
| `s14..s27` | NUM row (14)      |
| `s28..s41` | TOP row (14)      |
| `s42..s55` | HOME row (14)     |
| `s56..s69` | BOT row (14)      |
| `s70..s83` | EXT row (14)      |
| `s84..s89` | thumbs (6)        |

Within a 14-slot row: `[L: OUT pinky ring mid index inner INX] [R: INX inner index mid ring pinky OUT]`.
So e.g. on the TOP row, `s28`=left OUT, `s29..s33`=left core (pinky..inner),
`s34`=left INX, `s35`=right INX, `s36..s40`=right core (inner..pinky), `s41`=right OUT.

`KM_<layer>` supplies the 90 bindings **comma-separated** (one per slot); the board's
`LAYOUT()` macro receives them as its 90 positional parameters.

---

## 2. How a board consumes the master

Three pieces per board:

1. **`config/geometry/geom_<board>.h`** — the adapter, derived from the board's
   `matrix_transform` order. It defines:
   - **`POS_*`** symbols mapping each logical key to that board's physical position
     number (used by `combos.dtsi`/`behaviors.dtsi` — no raw number appears in
     `shared/`), and
   - a **`LAYOUT(...)`** macro taking the 90 master slots and emitting this board's
     `bindings` in its physical order, inserting `&none` for dead positions and
     ignoring slots the board doesn't have.

2. **`config/<board>.keymap`** — thin: includes the geometry header and `shared/*`,
   then maps each layer with `bindings = <KEYMAP_LAYER(KM_base)>` etc.

3. **`config/<board>.conf`** — board-specific overrides only (see §6).

### The `KEYMAP_LAYER` indirection (why it exists)

```c
#define KEYMAP_LAYER(...) LAYOUT(__VA_ARGS__)
...
base { bindings = <KEYMAP_LAYER(KM_base)>; };
```

The C preprocessor counts a macro's arguments **before** expanding them. Written as
`LAYOUT(KM_base)`, `LAYOUT` would see *one* argument. `KEYMAP_LAYER` forces `KM_base`
to expand to its 90 comma-separated slots first, so `LAYOUT` then receives 90
arguments. (This is the classic "argument prescan" trick.)

---

## 3. Adapters in detail

**Identity adapters (cb34s, Urchin).** These boards are a pure `3×5+2` core numbered
contiguously `0..33`, so their `LAYOUT()` just emits the core slots in order and
drops everything else. `geom_cb34s.h` is the template for any new `3×5+2` board.

**TOTEM adapter (dead + extra keys).** TOTEM's matrix has 38 physical positions:

```
 0  1  2  3  4   |   5  6  7  8  9        row 0
10 11 12 13 14   |  15 16 17 18 19        row 1 (home)
20 21 22 23 24 25|  26 27 28 29 30 31     row 2  (20 & 31 = outer pinkies)
        32 33 34 |  35 36 37              thumbs (32 & 37 = outer thumbs)
```

Its `LAYOUT()` maps the core to positions 1..4/etc. and — importantly — maps the
**outer pinkies (20, 31)** to the master `OUT` column bottom-row slots (`s56`,`s69`)
and the **outer thumbs (32, 37)** to the master `TH_O` slots (`s84`,`s89`). So those
four physical keys are *master-driven*: if you later put content in those master
slots, TOTEM picks it up automatically. They also get `POS_*` symbols
(`POS_LBX`,`POS_RBX`,`POS_LHX`,`POS_RHX`) so combos/HRM can target them.

> Because those four slots are currently reserved, TOTEM emits `&none` on `base`
> (unchanged) and `&trans` on higher layers there — both do nothing, so behaviour is
> unchanged today, but the wiring is ready.

---

## 4. Editing the master

- **Change a key / combo / macro / HRM:** edit `config/shared/*`. Never edit a thin
  `config/<board>.keymap`.
- **Populate an optional slot** (e.g. give boards with an outer column a bracket):
  put the binding in that slot of the relevant `KM_<layer>` in `master_layers.dtsi`.
  Boards whose adapter maps that slot get it; boards that don't, ignore it.
- **`&none` vs `&trans`:** on `base`, an unused optional slot is `&none`. On higher
  layers it is `&trans` (so a *populated* base key falls through instead of dying).
- **Layer-access keys** (`&lt`/`&mo`/`&tog`) must sit on core positions every board
  has (the inner thumbs / core rows), so every board can reach every layer.

### Combos & HomeRowMods

`config/shared/combos.dtsi` and `config/shared/behaviors.dtsi` reference only `POS_*`
symbols. Combos that touch an **optional** slot must be guarded so boards without
that key are unaffected, e.g.:

```c
#ifdef HAS_OUTER_COL
   Cmb_outer { key-positions = <POS_LBX ...>; ... };
#endif
```

(No such guard is needed yet — every current combo lives in the core.)

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

Common settings live once in **`config/shared.conf`** (modelled on the TOTEM options)
and are applied to every real board build via `-DEXTRA_CONF_FILE=../../config/shared.conf`
in `build.yaml`. A per-board **`config/<board>.conf`** exists **only** when a board
needs something specific (e.g. `cb34s.conf` carries the nice!view display widgets);
it is merged on top of `shared.conf`. `settings_reset` builds deliberately omit the
shared conf.

---

## 7. Verification

Correctness is proven by preprocessing, not just "it builds":

- **`scratchpad/verify.py`** runs the C preprocessor over the *original*
  `zmk-config-totem` keymap and our new `totem.keymap`, then compares every layer's
  expanded `bindings` and every combo. TOTEM must stay **functionally equivalent**:
  `base` byte-identical, and higher layers identical except the inert `&none`↔`&trans`
  at the four reserved outer positions (20/31/32/37). All 37 combos identical.
- **cb34s / Urchin** must preprocess to exactly 34 bindings/layer.
- **CI** (GitHub Actions, `build.yaml`) then builds every board + a `settings_reset`
  image against `zmk main`.

Adding a board or growing the master beyond `6×7+3` is described in
[`ADDING_A_BOARD.md`](../ADDING_A_BOARD.md).
