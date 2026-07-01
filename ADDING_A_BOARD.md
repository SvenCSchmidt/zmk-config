# Adding a board

This repo keeps **all** layer content, combos, macros, HomeRowMods and settings in
`config/shared/`. A board never gets its own content — it only gets a *geometry
adapter* that places the shared master layers onto its physical matrix. Adding a
board therefore touches no shared file.

## Steps

1. **Provide the hardware definition.**
   - Prefer an upstream module referenced from `config/west.yml`.
   - Otherwise copy the board/shield definition under `boards/` (this repo uses
     `board_root: .`, see `zephyr/module.yml`). For a shield that means
     `boards/shields/<name>/` with its `*.overlay`/`*.dtsi`, `Kconfig.shield`,
     `Kconfig.defconfig`, and (for splits) `<name>.zmk.yml`.

2. **Create `config/geometry/geom_<board>.h`** with two things, both read off the
   board's `matrix_transform` order:
   - `POS_*` defines mapping each logical core position to this board's physical
     number (see `geom_cb34s.h` for a pure 3x5+2 template, `geom_totem.h` for a
     board with dead/extra physical positions).
   - a `LAYOUT(...)` macro taking the 90 canonical master slots (the full 6x7+3
     grid; order documented in `config/shared/master_layers.dtsi`) and emitting this
     board's `bindings` order. Emit only the master slots this board physically has
     (e.g. a 3x5+2 board uses only the core slots), and insert `&none` for any dead
     physical positions. Most boards ignore the majority of the 90 slots.
   - Only reuse another board's header via `#include` if the two boards have a
     **provably identical** matrix order.

3. **Create the thin `config/<board>.keymap`** (copy `config/totem.keymap`): include
   the new geometry header, then `config/shared/*`, then map each layer with
   `bindings = <KEYMAP_LAYER(KM_<layer>)>`. Add board-specific `sensor-bindings`
   (encoders) or combo `#ifdef` guards only here. Add a matching `config/<board>.conf`.

4. **Add the board to `build.yaml`** (and a `settings_reset` build for its hardware).

## Populating optional slots / going bigger than the master

The master is a full **6 rows x 7 columns per hand + 3 thumbs per hand** grid
(90 slots). The middle 5 columns x 3 rows + inner 2 thumbs are the shared core
(populated); every other slot (the `OUT`/`INX` columns, the `FN`/`NUM`/`EXT` rows,
the `TH_O` thumbs) is reserved and currently `&none`/`&trans`.

- A board that **has** one of those optional keys: fill that slot in the relevant
  `KM_<layer>` in `master_layers.dtsi` with real content (still `&none`/`&trans`
  where a layer shouldn't use it), and map it in the board's `LAYOUT()`. Combos
  that touch an optional slot must be guarded (e.g. `#ifdef HAS_OUTER_COL`) so
  boards without that key are unaffected.
- A board **bigger** than 6x7+3 (more rows/columns/thumbs, or a center/encoder-press
  key) is the rare case that grows the master itself. Append the new slot group at
  the **end** of every `KM_<layer>` and the `LAYOUT()` signature so existing boards'
  adapters stay valid unchanged. A center key is the natural first such addition.
