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
   - a `LAYOUT(...)` macro taking the 34 canonical master slots (order documented
     in `config/shared/master_layers.dtsi`) and emitting this board's `bindings`
     order, inserting `&none` for physical positions the master doesn't fill and
     dropping master slots the board doesn't have.
   - Only reuse another board's header via `#include` if the two boards have a
     **provably identical** matrix order.

3. **Create the thin `config/<board>.keymap`** (copy `config/totem.keymap`): include
   the new geometry header, then `config/shared/*`, then map each layer with
   `bindings = <LAYOUT(KM_<layer>)>`. Add board-specific `sensor-bindings`
   (encoders) or combo `#ifdef` guards only here. Add a matching `config/<board>.conf`.

4. **Add the board to `build.yaml`** (and a `settings_reset` build for its hardware).

## If the board is bigger than the master

The master is currently a 34-key 3x5+2 core. A board that needs an outer column,
a 3rd thumb, a number row or a center/encoder key requires those **optional slot
groups** to be appended to each `KM_<layer>` in `master_layers.dtsi` (default
`&none` on base, `&trans` on higher layers) and to the `LAYOUT()` signature.
Append new slots at the end so existing boards' adapters stay valid. Combos that
touch optional slots must be guarded (e.g. `#ifdef HAS_OUTER_COL`) so boards
without them are unaffected.
