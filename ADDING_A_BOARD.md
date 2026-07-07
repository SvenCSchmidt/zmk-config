# Adding a board

> 🇩🇪 **Diese Anleitung gibt es auch auf Deutsch: [`ADDING_A_BOARD.de.md`](ADDING_A_BOARD.de.md).**

This repo keeps **all** layer content, combos, macros, HomeRowMods and settings in
`config/shared/`. A board never gets its own content — it only gets a *geometry
adapter* that weaves the shared core (and any reusable add-ons) onto its physical
matrix. Adding a board therefore touches no shared file.

See [`docs/MASTER_LAYOUT.md`](docs/MASTER_LAYOUT.md) for the full model; this is the
short recipe. For the **complete reference with a full worked example** (a fictional
board larger than `3×5+2`, incl. an override and an encoder) and every variable
documented, see [`docs/REFERENCE.en.md`](docs/REFERENCE.en.md) (English) /
[`docs/REFERENCE.de.md`](docs/REFERENCE.de.md) (Deutsch).

## Steps

1. **Provide the hardware definition.**
   - Prefer an upstream module referenced from `config/west.yml`.
   - Otherwise vendor the definition under `boards/shields/<name>/` (this repo uses
     `board_root: .`, see `zephyr/module.yml`; that one folder holds both plain shields
     and full HWv2 controller boards). For a shield that means its `*.overlay`/`*.dtsi`,
     `Kconfig.shield`, `Kconfig.defconfig`, and (for splits) `<name>.zmk.yml`; for a full
     board its `board.yml`, `Kconfig.<board>`, `<board>_<soc>_zmk.dts`, defconfig,
     `board.cmake`, etc. Vendor **only** the hardware files — never the source repo's own
     `keymap`/`keys_*`/`.conf` radio settings (those come from `config/shared/` and the
     shared confs).

2. **Pick or write the geometry adapter.** Read the board's `matrix_transform` order,
   then:
   - **If it's a common contiguous shape, reuse a shared adapter** — no new file:
     `#include "geometry/geom_3x5_2.h"` for a pure `3×5 + 2 thumbs/hand` (positions
     `0..33`), or `geometry/geom_3x5_3.h` for `3×5 + 3 thumbs/hand` (`0..35`, Variant A:
     inner two thumbs are the core, outer is the shared 3rd thumb).
   - **Otherwise create `config/geometry/geom_<board>.h`** with two things, both read
     off the matrix order:
     - `POS_*` defines mapping each logical core position to this board's physical
       number (`POS_LT0..POS_RB0`, `POS_LH0/LH1`, `POS_RH0/RH1`, plus any extras).
       `combos.dtsi`/`behaviors.dtsi` use only these symbols — no raw number in `shared/`.
     - a `KEYMAP_LAYER(L)` macro weaving the core fragments into this board's order:
       start from `WEAVE_ALPHA_3x5(L)` (30 alpha keys) or `WEAVE_CORE(L)` (34 = alpha +
       inner 2 thumbs), then add the board's extras. Use `RSVD_##L` for a physical key
       whose content isn't decided yet (`&none` base / `&trans` higher).
     - Templates: `geom_totem.h` (extra pinky + 3rd thumb via the shared add-ons),
       `geom_cornholius.h` (outer column + hand-written 4th row), `geom_chiffre.h`
       (center key spliced into the bottom row).

3. **Opt into add-ons for shared extras.** If the board has an outer pinky column or a
   3rd thumb, `#include "../shared/addons/outer_col.h"` / `thumb_outer.h` and weave
   `OUTER_<layer>_<row>_<hand>` / `THUMB_O_<layer>_<hand>` at those positions. To change
   just this board's version of a shared key, `#define` that fragment **before** the
   include (the guards make the board's value win).

4. **Create the thin `config/<board>.keymap`** (copy an existing one, e.g.
   `config/mnhttn.keymap`): include the geometry header, then `config/shared/*`, then
   map each layer with `bindings = <KEYMAP_LAYER(base)>` etc. Add board-specific
   `sensor-bindings` (encoders) or combo `#ifdef` guards only here. Add a
   `config/<board>.conf` only if the board needs something specific (a display, …).

5. **Add the board to `build.yaml`** with the right `-DEXTRA_CONF_FILE`
   (`shared_ble.conf` for wireless, `shared_usb.conf` for wired RP2040/USB), plus a
   `settings_reset` build for its hardware. Encoder `EC11` config lives in the shield's
   own `<shield>.conf`.

6. **Verify** before pushing: run `scratchpad/verify.py` (TOTEM's core must stay
   byte-identical — proves you didn't disturb shared content) and a binding-count check
   for the new board (it must preprocess to exactly the number of keys its matrix has;
   copy `scratchpad/verify_endgame.py` and change the expected count).

## Activating a shared extra everywhere

To give *every* board that has an outer column a new key, edit the add-on
(`config/shared/addons/outer_col.h`) once — it reaches all of them. A combo that
touches such an optional position must be guarded (e.g. `#ifdef HAS_OUTER_COL`) so
boards without that key are unaffected.
