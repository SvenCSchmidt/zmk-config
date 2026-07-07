<!--
  This document is also available in English: docs/MASTER_LAYOUT.md
-->
# Das Master-Layout — wie dieses Repo funktioniert

> 🇬🇧 **This document is also available in English: [`MASTER_LAYOUT.md`](MASTER_LAYOUT.md).**

> Dies ist der konzeptionelle Überblick. Für die **vollständige Referenz** — jede
> geteilte Variable dokumentiert und ein komplettes Schritt-für-Schritt-Board-Beispiel —
> siehe [`REFERENCE.de.md`](REFERENCE.de.md) (Deutsch) / [`REFERENCE.en.md`](REFERENCE.en.md)
> (English).

Dieses Repo kompiliert **eine** Keymap für **viele** Tastaturen. Aller geteilte Inhalt
liegt einmalig in `config/shared/`; jedes Board trägt nur einen dünnen
*Geometrie-Adapter* (`config/geometry/geom_<board>.h`), der diesen Inhalt zur
Compile-Zeit auf seine physischen Tasten webt.

> TL;DR-Denkmodell: Der geteilte **Core** (3x5 + 2 Daumen) wird einmal als kleine
> Blöcke je Layer/Reihe/Hand in `config/shared/core_blocks.dtsi` definiert.
> Wiederverwendbare **AddOns** (`config/shared/addons/*.h`) enthalten zusätzliche
> Strukturen, die mehrere Boards teilen (die äußere Pinky-Spalte, den 3. Daumen). Jedes
> `geom_<board>.h` wählt, welche davon ein Board physisch hat, und webt sie in seiner
> Verdrahtungsreihenfolge; alles, was ein Board nicht teilt, schreibt es von Hand.
> Geteilten Inhalt zu bearbeiten ändert jedes betroffene Board auf einmal.

### Einmal ändern → genau eine Datei bearbeiten

Der ganze Sinn: Eine Änderung, die viele Boards betrifft, fasst **eine** Datei an.

| Was du änderst | Die eine zu bearbeitende Datei | Erreicht |
|---|---|---|
| Eine Core-Taste / HomeRowMod / Core-Daumen | `config/shared/core_blocks.dtsi` | jedes Board |
| Ein Combo | `config/shared/combos.dtsi` | jedes Board |
| Ein Makro | `config/shared/macros.dtsi` | jedes Board |
| Ein Behavior (HRM-Timing, Cadet-Shift) | `config/shared/behaviors.dtsi` | jedes Board |
| Die äußere Pinky-Spalte (Esc/Tab/…) | `config/shared/addons/outer_col.h` | Corne, Cornholius, Splaytoraid |
| Der 3. (äußere) Daumen | `config/shared/addons/thumb_outer.h` | jedes Board, das einen hat |
| Gemeinsame BLE-Einstellungen | `config/shared_ble.conf` | jedes kabellose Board |
| Gemeinsame USB-Einstellungen | `config/shared_usb.conf` | jedes kabelgebundene RP2040-Board |

Ein `geom_<board>.h` enthält **nur** die physischen Positionsnummern dieses Boards
(seine Verdrahtung) und seine Weave-Reihenfolge — du fasst es für eine Inhaltsänderung
nie an, sondern nur, wenn ein Board hinzugefügt oder neu verdrahtet wird. So wird kein
geteilter Inhalt jemals über Boards hinweg dupliziert.

---

## 1. Der geteilte Core (`core_blocks.dtsi`)

`config/shared/core_blocks.dtsi` ist die Single Source of Truth für den Teil, den
*jedes* Board gemeinsam hat: den **3×5-Alphablock + die inneren 2 Daumen je Hand** (34
Tasten) mit dem kanonischen Colemak-DH-Inhalt. Es ist **kein** großes Raster — es ist
eine Menge kleiner, kombinierbarer Fragmente, eines je Layer × Reihe × Hand, die sich
jeweils zu einer leerzeichengetrennten Liste von ZMK-Bindings expandieren (keine
Kommata, sodass jedes direkt in ein `bindings = <...>`-Array passt):

```
 CORE_<layer>_top_L    CORE_<layer>_top_R      Reihe 0 (oben, Alpha)  Q W F P B | J L U Y ;
 CORE_<layer>_home_L   CORE_<layer>_home_R     Reihe 1 (home + HRMs)  A R S T G | M N E I O
 CORE_<layer>_bot_L    CORE_<layer>_bot_R      Reihe 2 (unten, Alpha) Z X C D V | K H , . /
 CORE_<layer>_thumb_L  CORE_<layer>_thumb_R    innere 2 Daumen/Hand
```

`<layer> ∈ { base, nav, num, fun, pad }`. Zwei Weave-Helfer setzen die Fragmente in
kanonischer Lesereihenfolge zusammen:

- **`WEAVE_ALPHA_3x5(L)`** — nur der 30-Tasten-Alphablock (oben, home, unten; je Reihe
  linke dann rechte Hand), **keine Daumen**. Boards weben ihre eigene Daumenanordnung
  danach.
- **`WEAVE_CORE(L)`** — `WEAVE_ALPHA_3x5(L)` **plus** die inneren 2 Daumen/Hand = der
  vollständige 34-Tasten-`3×5+2`-Core.

Es gibt **kein** reserviertes äußeres Raster, keine Funktions- oder Zahlenreihe, die in
den Core eingebacken wäre. Alles jenseits der 34 Tasten ist entweder ein
wiederverwendbares **AddOn** (§2), das ein Board einbindet, oder von Hand im Adapter des
Boards geschrieben (§3). Ein Board, das eine Zusatztaste physisch hat, sie aber noch
nicht belegt hat, webt dort den Platzhalter **`RSVD_<layer>`** (`&none` auf base,
`&trans` auf höheren Layern), sodass die Taste inaktiv, aber verdrahtet ist.

---

## 2. Wiederverwendbare AddOns (`config/shared/addons/*.h`)

Strukturen, die mehrere — aber nicht alle — Boards teilen, liegen einmalig als AddOns
vor. Jedes AddOn definiert Fragmente in derselben Form wie der Core (je Layer ×
Position), und **jedes Fragment ist `#ifndef`-geschützt**, sodass ein Board jede einzelne
Position überschreiben kann, indem es sie *vor* dem Include `#define`t.

| AddOn | Fragmente | Standardinhalt (base) | Genutzt von |
|---|---|---|---|
| `addons/outer_col.h` | `OUTER_<layer>_<reihe>_<hand>` (top/home/bot × L/R) | L: `Esc` / `-` / Cadet-Shift `(` — R: `'` / `\` / Cadet-Shift `)` | Corne, Cornholius, Splaytoraid |
| `addons/thumb_outer.h` | `THUMB_O_<layer>_<hand>` (der 3./äußere Daumen) | ungenutzt (`&none`) | jedes Board mit 3. Daumen |

**Override-Konvention** — Board-Overrides → AddOn → Core. Beispiel: Cornholius' äußere
Spalte weicht in nur drei base-Positionen vom Standard ab:

```c
#define OUTER_base_home_L  &kp TAB    /* Standard war  -  */
#define OUTER_base_top_R   &kp BSPC   /* Standard war  '  */
#define OUTER_base_home_R  &kp RET    /* Standard war  \  */
#include "../shared/addons/outer_col.h"   /* füllt die übrigen Positionen */
```

Da die Guards das `#define` des Boards gewinnen lassen, erreicht das Bearbeiten von
`outer_col.h` weiterhin jede *nicht überschriebene* Position auf jedem Board, das es
einbindet. Cadet-Shift `(`/`)` (Halten = Shift, Tippen = Klammer) benötigt das
`cadet`-Hold-Tap aus `shared/behaviors.dtsi`.

---

## 3. Der board-eigene Geometrie-Adapter (`geom_<board>.h`)

Jedes Board hat einen Adapter-Header mit zwei Artefakten, beide aus der
`matrix_transform`-Reihenfolge des Boards abgeleitet:

1. **`POS_*`**-Symbole, die jede logische Core-Taste auf die physische Positionsnummer
   des Boards abbilden (`POS_LT0..POS_RB0`, `POS_LH0/LH1`, `POS_RH0/RH1`, plus etwaige
   Extras wie `POS_LHX`/`POS_CENTER`). `combos.dtsi` und `behaviors.dtsi` referenzieren
   nur diese Symbole — **nie erscheint eine rohe Positionsnummer in `shared/`.**
2. **`KEYMAP_LAYER(L)`** — webt die Core-Fragmente (und etwaige AddOns / handgeschriebene
   Ränder) in die physische Reihenfolge des Boards. Die dünne Keymap schreibt dann nur
   `bindings = <KEYMAP_LAYER(base)>` je Layer.

### Geteilte Adapter für die häufigen Formen

Die meisten Boards haben eine von zwei lückenlosen Formen und brauchen keinen eigenen
Adapter — sie `#include`n einen geteilten Geometrie-Header:

- **`geometry/geom_3x5_2.h`** — reines `3×5 + 2 Daumen/Hand`, nummeriert `0..33`
  (`KEYMAP_LAYER(L) = WEAVE_CORE(L)`). Boards: cb34s, Urchin, re-gret, delta-omega,
  sweep. Seine Alpha-`POS_*` kommen aus `geometry/core_3x5.h` (Positionen `0..29`),
  geteilt, damit diese 30 Nummern einmal definiert sind.
- **`geometry/geom_3x5_3.h`** — `3×5 + 3 Daumen/Hand`, nummeriert `0..35` (fügt den
  äußeren Daumen über `thumb_outer.h` hinzu; die inneren zwei sind der Core — „Variante
  A“). Boards: MNHTTN, Onyx Cinder, Endgame/BLEndgame.

### Eigene Adapter (Supersets & ungewöhnliche Matrizen)

Ein Board schreibt sein eigenes `KEYMAP_LAYER`, wenn seine Matrix keine dieser Formen
ist — es webt die Core-Fragmente plus das, was es sonst hat:

- **TOTEM** (`geom_totem.h`) — ein `3×5`-Core plus, je Hand, ein Extra-Pinky in der
  unteren Reihe und ein 3. (äußerer) Daumen. Beide nutzen die geteilten AddOns: Der
  Extra-Pinky ist die untere Taste der äußeren Spalte (`OUTER_*_bot`, standardmäßig
  Cadet-Shift) und der 3. Daumen ist `THUMB_O_*` (standardmäßig ungenutzt).
- **Corne / Cornholius** (`geom_corne.h`, `geom_cornholius.h`) — äußere Pinky-Spalte
  (`OUTER_*`) um den Core auf allen drei Reihen, plus der 3. Daumen (`THUMB_O_*`);
  Cornholius fügt eine handgeschriebene 4. Reihe äußerer Modifier (`C_R3_*`) hinzu.
- **Le Chiffre** (`geom_chiffre.h`) — `3×5` mit einer **Mitteltaste** (`CHIFF_CENTER`,
  der Encoder-Druck = Mute), die in die Mitte der unteren Reihe eingesetzt ist und den
  unteren rechten Core auf `26..30` verschiebt — weshalb es `core_3x5.h` nicht
  wiederverwenden kann.
- **Splaytoraid** (`geom_splaytoraid.h` / `geom_splaytoraid36.h`) — Core + eine
  Encoder-Druck-Mitteltaste + 3 Daumen/Hand, mit (40) oder ohne (36) die äußere Spalte.

### Warum der Name `KEYMAP_LAYER` existiert (Argument-Prescan)

`KEYMAP_LAYER(L)` ist ein einfacher objektartiger Weave, aber die Indirektion beizubehalten
ist wichtig: Der C-Präprozessor fügt `##`-Token zusammen und expandiert Fragment-Makros
in definierter Reihenfolge, sodass `KEYMAP_LAYER(base)` jedes `CORE_base_*`- /
`OUTER_base_*`-Fragment zu seinen Bindings expandiert, *bevor* das Devicetree das
`<...>`-Array sieht. Jedes expandierte Fragment ist leerzeichengetrennt (nie
kommagetrennt), sodass der gesamte Layer eine flache Binding-Liste wird, egal aus wie
vielen Fragmenten/AddOns ein Board sie zusammengesetzt hat.

---

## 4. Inhalt bearbeiten — die Ein-Datei-Regel

- **Eine Core-Taste / HRM / Core-Daumen ändern:** `config/shared/core_blocks.dtsi`
  bearbeiten. Nie eine dünne `config/<board>.keymap`.
- **Die äußere Spalte oder den 3. Daumen** für alle ändern, die sie haben: das AddOn
  bearbeiten (`addons/outer_col.h` / `addons/thumb_outer.h`). Um es für *ein* Board zu
  ändern, diese Position im `geom_<board>.h` des Boards vor dem Include `#define`n.
- **`&none` vs. `&trans`:** Ungenutzte Ränder nutzen `RSVD_<layer>` — `&none` auf base
  (die Taste tut nichts), `&trans` auf höheren Layern (eine belegte base-Taste fällt
  durch statt zu sterben). AddOns folgen für ihre höheren Layer derselben Regel.
- **Layer-Zugriffstasten** (`&lt`/`&mo`/`&tog`) liegen auf Core-Positionen, die jedes
  Board hat (die inneren Daumen / Core-Reihen), damit jedes Board jeden Layer erreicht.

### Combos & HomeRowMods

`config/shared/combos.dtsi` und `config/shared/behaviors.dtsi` referenzieren nur
`POS_*`-Symbole, sind also board-unabhängig. Ein Combo, das eine **optionale**
(Nicht-Core-)Position berührt, muss geschützt werden, damit Boards ohne diese Taste
unberührt bleiben, z. B.:

```c
#ifdef HAS_OUTER_COL
   Cmb_outer { key-positions = <POS_LOB ...>; ... };
#endif
```

(Ein solcher Guard ist heute nicht nötig — jedes aktuelle Combo liegt im Core.)

---

## 5. Board-Hardware: Module vs. vendort, und alte Repos löschen

Die Hardwaredefinition eines Boards (Shield: `matrix_transform`, kscan, Overlays) kann
aus zwei Quellen stammen:

- **Upstream-ZMK-Modul** (bevorzugt) — in `config/west.yml` referenziert; die CI holt
  die echten Dateien zur Build-Zeit. Beispiel: **Urchin** nutzt `duckyb/urchin-zmk-module`.
- **Vendort** unter `boards/shields/<board>/` — eine lokale Kopie, genutzt, wenn kein
  passendes Upstream-Modul existiert oder das Modul nicht gegen unsere ZMK-Revision baut.

Aktuelle Bezugsquellen:

| Board  | Quelle | Warum |
|--------|--------|-------|
| Urchin | Modul `duckyb/urchin-zmk-module` | Upstream-Modul, baut auf `zmk main` |
| TOTEM  | vendort `boards/shields/totem/` | das Upstream-Modul `BildermanKawasaki/zmk-keyboard-TOTEM` ist auf ZMK v0.3 gepinnt und noch nicht Zephyr-4.1/`main`-kompatibel; die vendorte Kopie baut auf `main` und wir kontrollieren sie |
| cb34s  | vendort `boards/shields/cb34s/` | es existiert kein verteilbares ZMK-Modul — das Upstream `bubbleology/zmk-config-cb34s` ist selbst ein Config-Repo, das das Shield lokal vendort (und gegen den `AakashDabas/zmk@animated_widgets`-Fork baut); `bubbleology/CB34S` enthält nur die PCB/KiCad-Dateien |

> **Deine alten Board-Config-Repos zu löschen ist sicher.** Nichts in diesem Repo
> referenziert `zmk-config-totem` / `zmk-config-cb34s` — die Shield-Definitionen sind
> vendort und in sich geschlossen, und Inhalt liegt in `shared/`. TOTEM auf das
> Upstream-Modul umzustellen ist eine spätere Option, sobald jenes Modul Zephyr 4.1
> unterstützt.

---

## 6. Konfiguration (`.conf`)

Gemeinsame Einstellungen liegen einmalig in einer **geteilten conf**, je Board nach
Funktyp gewählt über `-DEXTRA_CONF_FILE` in `build.yaml`:

- **`config/shared_ble.conf`** — kabellose/BLE-Boards (nRF: nice!nano & Klone, xiao_ble).
  Aktiviert Bluetooth, Deep Sleep, Split-Akkumeldung, Entprellung und Combo-Limits.
- **`config/shared_usb.conf`** — kabelgebundene USB-only-Boards (RP2040-„Pro-Micro“-Klone
  wie Helios/SparkFun, z. B. Splaytoraid40). Gleiche Entprellung/Combo-Limits **ohne**
  Bluetooth (diese Boards haben keinen Funk, `CONFIG_BT` würde scheitern).

Eine board-eigene **`config/<board>.conf`** existiert **nur**, wenn ein Board etwas
Spezifisches braucht (z. B. trägt `cb34s.conf` die nice!view-Display-Widgets); sie wird
über die geteilte conf gemischt. Encoder-EC11-Konfig liegt in der eigenen
`<shield>.conf` des Shields. `settings_reset`-Builds lassen die geteilte conf bewusst weg.

---

## 7. Verifikation

Korrektheit wird durch Präprozessieren bewiesen, nicht nur durch „es baut“:

- **`scratchpad/verify.py`** lässt den C-Präprozessor über die *originale*
  `zmk-config-totem`-Keymap und unsere `totem.keymap` laufen und vergleicht dann die
  expandierten `bindings` jedes Layers sowie jedes Combo. TOTEMs **34-Tasten-Core muss
  byte-identisch bleiben** — das Regressions-Gate, das beweist, dass eine Änderung am
  geteilten Inhalt das kanonische Layout nicht verändert hat: 5/5 Layer identisch, alle
  37 Combos identisch. (TOTEMs vier äußere Tasten — die Extra-Pinkys + 3. Daumen — werden
  jetzt von den geteilten AddOns gesteuert und sind daher vom Byte-Vergleich ausgenommen.)
- **Board-Binding-Anzahl** (z. B. `scratchpad/verify_cb34s.py`, `verify_endgame.py`): die
  Keymap eines Boards muss zu exakt der Zahl an Bindings präprozessieren, die seine Matrix
  hat — `34` für einen `3×5+2`-Core, `36` für ein `3×5+3`-Board usw. Das fängt ein falsch
  gewebtes `KEYMAP_LAYER` (falsche Fragment-Reihenfolge, eine ausgelassene oder doppelte
  Position) vor der CI ab. Die Skripte nutzen den von `verify.py` erzeugten bereinigten
  Baum wieder.
- **CI** (GitHub Actions, `build.yaml`) baut dann jedes Board + ein
  `settings_reset`-Image gegen `zmk main`.

Das Hinzufügen eines Boards ist in [`ADDING_A_BOARD.de.md`](../ADDING_A_BOARD.de.md)
beschrieben: den passenden `geom_3x5_*`-Header wählen (oder einen kleinen
`geom_<board>.h`-Weave für eine ungewöhnliche Matrix schreiben), eine dünne Keymap und
einen `build.yaml`-Eintrag hinzufügen — keine Änderungen am geteilten Inhalt.
