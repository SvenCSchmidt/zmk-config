# zmk-config

One ZMK config for many keyboards. Keymap, combos, macros, HomeRowMods and
settings are defined **once** in `config/shared/`; each board only carries a thin
*geometry adapter*. Change a combo or a key in one place and rebuild firmware for
every board.

## How it works (Strategy B: master layout + per-board adapter)

- **`config/shared/master_layers.dtsi`** holds every layer's content as `KM_<layer>`
  macros over a fixed set of named *master slots*. The master is a maximal
  **6 rows x 7 columns per hand + 3 thumbs per hand** grid (90 slots); the middle
  5 columns x 3 rows + inner 2 thumbs are the shared core that is actually populated,
  and every other slot (outer/inner-extra columns, function/number/extra rows, outer
  thumb) is reserved for future boards (`&none`/`&trans`). This is the single source
  of truth, written as a readable key grid.
- **`config/geometry/geom_<board>.h`** provides, for one board:
  - `POS_*` symbols mapping logical key positions to that board's physical numbers
    (used by `combos.dtsi` and `behaviors.dtsi` — no raw number lives in `shared/`), and
  - a QMK-style `LAYOUT()` macro that places the master slots onto the board's
    physical matrix at compile time (reordering, padding dead positions with
    `&none`, dropping slots the board doesn't have).
- **`config/<board>.keymap`** is thin: it selects the geometry header, includes
  `config/shared/*`, and maps each layer with `bindings = <KEYMAP_LAYER(KM_base)>`.
- **`config/shared.conf`** holds the common Kconfig settings for every board
  (applied via `EXTRA_CONF_FILE` in `build.yaml`); a per-board `config/<board>.conf`
  exists only for board-specific overrides (e.g. a display).

The canonical content is the Colemak-DH TOTEM layout (`base`, `nav`, `num`, `fun`,
`pad`). Host layout assumption: the OS keyboard layout is **German (DE)** (umlauts/€
are produced via AltGr; see `config/shared/keys_de.h`).

**➜ Full details in [`docs/MASTER_LAYOUT.md`](docs/MASTER_LAYOUT.md)** — the master
grid and slot indices, naming, adapters, editing, board sourcing (module vs.
vendored), config, and verification.

## Boards

| Board  | Hardware                       | Geometry            |
|--------|--------------------------------|---------------------|
| TOTEM  | `xiao_ble//zmk` + `totem_left/right` | 3x5 + 2 thumbs/side |
| cb34s  | `nice_nano//zmk` + `cb34s`           | 3x5 + 2 thumbs/side |
| Urchin | `nice_nano//zmk` + `urchin_left/right` (module `duckyb/urchin-zmk-module`) | 3x5 + 2 thumbs/side |

Adding a board changes no shared content — see [`ADDING_A_BOARD.md`](ADDING_A_BOARD.md).

## Building

GitHub Actions builds every entry in [`build.yaml`](build.yaml) (each board plus a
`settings_reset` image) and publishes the `.uf2` files as workflow artifacts.

## Planned

- **Optional animated display per board.** It should be possible to opt a board
  into animated display widgets (the cb34s previously used the
  `AakashDabas/zmk@animated_widgets` fork for this). The plan is to gate it behind
  a per-board option — e.g. a dedicated `build.yaml` entry with an
  `animated_widgets` ZMK module/revision and the matching `.conf` flags — so only
  boards that ask for it pull in the animation code, while the default build stays
  on upstream `zmk main`.
