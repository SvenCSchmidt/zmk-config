<!--
  This guide is also available in English: ADDING_A_BOARD.md
-->
# Ein Board hinzufügen

> 🇬🇧 **This guide is also available in English: [`ADDING_A_BOARD.md`](ADDING_A_BOARD.md).**

Dieses Repo hält **allen** Layer-Inhalt, Combos, Makros, HomeRowMods und Einstellungen
in `config/shared/`. Ein Board bekommt nie eigenen Inhalt — es bekommt nur einen
*Geometrie-Adapter*, der den geteilten Core (und etwaige wiederverwendbare AddOns) auf
seine physische Matrix webt. Ein Board hinzuzufügen fasst daher keine geteilte Datei an.

Siehe [`docs/MASTER_LAYOUT.de.md`](docs/MASTER_LAYOUT.de.md) für das vollständige Modell;
dies ist das Kurzrezept. Für die **vollständige Referenz mit einem kompletten Beispiel**
(ein fiktives Board größer als `3×5+2`, inkl. Override und Encoder) und jede dokumentierte
Variable siehe [`docs/REFERENCE.de.md`](docs/REFERENCE.de.md) (Deutsch) /
[`docs/REFERENCE.en.md`](docs/REFERENCE.en.md) (English).

## Schritte

1. **Die Hardwaredefinition bereitstellen.**
   - Bevorzugt ein Upstream-Modul, referenziert aus `config/west.yml`.
   - Andernfalls die Definition unter `boards/shields/<name>/` vendoren (dieses Repo nutzt
     `board_root: .`, siehe `zephyr/module.yml`; dieser eine Ordner enthält sowohl reine
     Shields als auch vollständige HWv2-Controller-Boards). Für ein Shield heißt das seine
     `*.overlay`/`*.dtsi`, `Kconfig.shield`, `Kconfig.defconfig` und (für Splits)
     `<name>.zmk.yml`; für ein vollständiges Board seine `board.yml`, `Kconfig.<board>`,
     `<board>_<soc>_zmk.dts`, defconfig, `board.cmake` usw. Vendore **nur** die
     Hardwaredateien — nie die `keymap`/`keys_*`/`.conf`-Funkeinstellungen des Quell-Repos
     (die kommen aus `config/shared/` und den geteilten confs).

2. **Den Geometrie-Adapter wählen oder schreiben.** Lies die `matrix_transform`-Reihenfolge
   des Boards, dann:
   - **Ist es eine häufige, lückenlose Form, verwende einen geteilten Adapter** — keine
     neue Datei: `#include "geometry/geom_3x5_2.h"` für ein reines `3×5 + 2 Daumen/Hand`
     (Positionen `0..33`) oder `geometry/geom_3x5_3.h` für `3×5 + 3 Daumen/Hand` (`0..35`,
     Variante A: die inneren zwei Daumen sind der Core, der äußere ist der geteilte 3.
     Daumen).
   - **Andernfalls erstelle `config/geometry/geom_<board>.h`** mit zwei Dingen, beide aus
     der Matrix-Reihenfolge abgelesen:
     - `POS_*`-Defines, die jede logische Core-Position auf die physische Nummer dieses
       Boards abbilden (`POS_LT0..POS_RB0`, `POS_LH0/LH1`, `POS_RH0/RH1`, plus etwaige
       Extras). `combos.dtsi`/`behaviors.dtsi` nutzen nur diese Symbole — keine rohe
       Nummer in `shared/`.
     - ein `KEYMAP_LAYER(L)`-Makro, das die Core-Fragmente in die Reihenfolge dieses Boards
       webt: beginne mit `WEAVE_ALPHA_3x5(L)` (30 Alpha-Tasten) oder `WEAVE_CORE(L)` (34 =
       Alpha + innere 2 Daumen), dann füge die Extras des Boards hinzu. Nutze `RSVD_##L`
       für eine physische Taste, deren Inhalt noch nicht feststeht (`&none` base / `&trans`
       höher).
     - Vorlagen: `geom_totem.h` (Extra-Pinky/3. Daumen über die geteilten AddOns),
       `geom_cornholius.h` (äußere Spalte + handgeschriebene 4. Reihe), `geom_chiffre.h`
       (Mitteltaste in die untere Reihe eingesetzt).

3. **AddOns für geteilte Extras einbinden.** Hat das Board eine äußere Pinky-Spalte oder
   einen 3. Daumen, `#include "../shared/addons/outer_col.h"` / `thumb_outer.h` und webe
   `OUTER_<layer>_<reihe>_<hand>` / `THUMB_O_<layer>_<hand>` an diesen Positionen. Um nur
   die Version dieses Boards einer geteilten Taste zu ändern, dieses Fragment **vor** dem
   Include `#define`n (die Guards lassen den Wert des Boards gewinnen).

4. **Die dünne `config/<board>.keymap` erstellen** (eine bestehende kopieren, z. B.
   `config/mnhttn.keymap`): den Geometrie-Header einbinden, dann `config/shared/*`, dann
   jeden Layer mit `bindings = <KEYMAP_LAYER(base)>` usw. abbilden. Board-spezifische
   `sensor-bindings` (Encoder) oder Combo-`#ifdef`-Guards nur hier ergänzen. Eine
   `config/<board>.conf` nur, wenn das Board etwas Spezifisches braucht (ein Display, …).

5. **Das Board zu `build.yaml` hinzufügen** mit dem richtigen `-DEXTRA_CONF_FILE`
   (`shared_ble.conf` für kabellos, `shared_usb.conf` für kabelgebundenes RP2040/USB),
   plus einem `settings_reset`-Build für seine Hardware. Encoder-`EC11`-Konfig liegt in der
   eigenen `<shield>.conf` des Shields.

6. **Verifizieren** vor dem Push: `scratchpad/verify.py` ausführen (TOTEMs Core muss
   byte-identisch bleiben — beweist, dass du den geteilten Inhalt nicht gestört hast) und
   eine Binding-Anzahl-Prüfung für das neue Board (es muss zu exakt der Anzahl Tasten
   präprozessieren, die seine Matrix hat; kopiere `scratchpad/verify_endgame.py` und ändere
   die erwartete Anzahl).

## Ein geteiltes Extra überall aktivieren

Um *jedem* Board mit äußerer Spalte eine neue Taste zu geben, bearbeite das AddOn
(`config/shared/addons/outer_col.h`) einmal — es erreicht sie alle. Ein Combo, das eine
solche optionale Position berührt, muss geschützt werden (z. B. `#ifdef HAS_OUTER_COL`),
damit Boards ohne diese Taste unberührt bleiben.
