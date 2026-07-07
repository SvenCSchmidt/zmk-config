<!--
  Dieses Dokument gibt es auch auf Englisch: docs/REFERENCE.en.md
-->
# Vollständige Referenz & Anleitung zum Hinzufügen von Boards (Deutsch)

> 🇬🇧 **This documentation is also available in English: [`REFERENCE.en.md`](REFERENCE.en.md).**

Dies ist die vollständige Referenz für die konsolidierte ZMK-Konfiguration: die gesamte
Architektur, **jede** geteilte Variable/Makro dokumentiert, eine **Schritt-für-Schritt-
Anleitung zum Hinzufügen eines Boards** mit einem vollständigen Beispiel (ein fiktives
Board größer als `3×5+2`, inklusive AddOn-Override und Encoder) sowie der
Verifikations-Workflow.

Für den kürzeren konzeptionellen Überblick siehe [`MASTER_LAYOUT.md`](MASTER_LAYOUT.md)
(Englisch); für die Kurz-Checkliste [`ADDING_A_BOARD.md`](../ADDING_A_BOARD.md)
(Englisch). Diese Datei ist die maßgebliche, vollständige Fassung.

---

## Inhaltsverzeichnis

1. [Grundidee](#1-grundidee)
2. [Repository-/Datei-Übersicht](#2-repository-datei-übersicht)
3. [Koordinatensystem: der logische Core](#3-koordinatensystem-der-logische-core)
4. [Variablen- & Makro-Katalog](#4-variablen--makro-katalog)
   - [4.1 Positions-Symbole `POS_*`](#41-positions-symbole-pos_)
   - [4.2 Core-Inhaltsfragmente `CORE_*`](#42-core-inhaltsfragmente-core_)
   - [4.3 Weave-Helfer `WEAVE_*` und `RSVD_*`](#43-weave-helfer-weave_-und-rsvd_)
   - [4.4 Das Weave-Makro `KEYMAP_LAYER`](#44-das-weave-makro-keymap_layer)
   - [4.5 AddOn: äußere Pinky-Spalte `OUTER_*`](#45-addon-äußere-pinky-spalte-outer_)
   - [4.6 AddOn: 3. Daumen `THUMB_O_*`](#46-addon-3-daumen-thumb_o_)
   - [4.7 Board-lokale Weave-Variablen](#47-board-lokale-weave-variablen)
   - [4.8 Behaviors (HomeRowMods + Cadet-Shift)](#48-behaviors-homerowmods--cadet-shift)
   - [4.9 Makros `MCR_*`](#49-makros-mcr_)
   - [4.10 Deutsche Aliase `DE_*`](#410-deutsche-aliase-de_)
   - [4.11 Combos](#411-combos)
   - [4.12 Globale Einstellungen](#412-globale-einstellungen)
   - [4.13 Kconfig-(`.conf`-)Variablen](#413-kconfig-conf-variablen)
   - [4.14 Encoder (`sensor-bindings`)](#414-encoder-sensor-bindings)
5. [Die fünf Layer](#5-die-fünf-layer)
6. [Schritt für Schritt: ein Board hinzufügen (vollständiges Beispiel)](#6-schritt-für-schritt-ein-board-hinzufügen-vollständiges-beispiel)
7. [Verifikation](#7-verifikation)
8. [Board-Hardware: Bezugsquellen](#8-board-hardware-bezugsquellen)

---

## 1. Grundidee

Eine Keymap, viele Tastaturen. Sämtlicher Inhalt wird **einmal** in `config/shared/`
definiert; jedes Board trägt nur einen dünnen *Geometrie-Adapter*, der diesen Inhalt zur
Compile-Zeit auf seine physischen Tasten webt (reiner C-Präprozessor, kein Build-Schritt).

- Der **geteilte Core** — ein `3×5`-Alphablock + die inneren 2 Daumen je Hand (34
  Tasten) — ist der Teil, den jede Tastatur hat. Er liegt in
  `config/shared/core_blocks.dtsi` als kleine, kombinierbare Fragmente.
- **Wiederverwendbare AddOns** (`config/shared/addons/*.h`) enthalten Strukturen, die
  *mehrere* Boards teilen (die äußere Pinky-Spalte, der 3. Daumen). Ein Board bindet die
  ein, die es physisch hat, und kann jede einzelne Taste **überschreiben**.
- Die **`config/geometry/geom_<board>.h`** jedes Boards definiert seine physischen
  Positionsnummern und ein Makro `KEYMAP_LAYER(L)`, das den Core (+ AddOns + evtl.
  handgeschriebene Randtasten) in die Matrix-Reihenfolge des Boards einfügt.
- Die **`config/<board>.keymap`** jedes Boards ist dünn: Geometrie-Header + geteilte
  Dateien einbinden, dann je Layer `bindings = <KEYMAP_LAYER(base)>` schreiben.

**Einmal ändern → überall neu bauen.** Ändere eine Core-Taste in `core_blocks.dtsi`, und
jedes Board übernimmt sie; ändere ein AddOn, und jedes Board, das es einbindet,
übernimmt es.

---

## 2. Repository-/Datei-Übersicht

```
zmk-config/
├── config/
│   ├── west.yml                     # einheitliche ZMK-Revision + Shield-/Widget-Module
│   ├── shared_ble.conf              # gemeinsame Kconfig für kabellose (BLE) Boards
│   ├── shared_usb.conf              # gemeinsame Kconfig für kabelgebundene USB-Boards
│   ├── <board>.keymap               # dünne Keymap je Board (eine pro Board)
│   ├── <board>.conf                 # OPTIONALE board-spezifische Kconfig (z. B. Display)
│   ├── shared/
│   │   ├── core_blocks.dtsi         # DER geteilte 3x5+2-Core-Inhalt (Single Source)
│   │   ├── behaviors.dtsi           # HomeRowMods + Cadet-Shift Hold-Taps
│   │   ├── macros.dtsi              # MCR_*-Makros
│   │   ├── combos.dtsi              # alle Combos (symbolische POS_*-Positionen)
│   │   ├── settings.dtsi            # globale Behavior-Einstellungen (caps_word-Liste)
│   │   ├── keys_de.h                # Aliase für das DE-Layout (DE_*)
│   │   └── addons/
│   │       ├── outer_col.h          # wiederverwendbare äußere Pinky-Spalte (OUTER_*)
│   │       └── thumb_outer.h        # wiederverwendbarer 3. Daumen (THUMB_O_*)
│   └── geometry/
│       ├── core_3x5.h               # geteilte Alpha-POS_* (0..29) für lückenlose Boards
│       ├── geom_3x5_2.h             # geteilter Adapter: reines 3x5 + 2 Daumen (0..33)
│       ├── geom_3x5_3.h             # geteilter Adapter: 3x5 + 3 Daumen (0..35)
│       └── geom_<board>.h           # board-eigene Adapter (Supersets, ungewöhnl. Matrix)
├── boards/
│   └── shields/<name>/              # alle vendorte Hardware: Split-/Shield-Definitionen
│                                    # UND vollständige HWv2-Controller-Boards (nRF/STM32)
├── drivers/                         # vendorte Out-of-Tree-Treiber (z. B. display/jd79653.c,
│                                    # das Tipper-TF-Epaper); nur bei Opt-in kompiliert
├── dts/bindings/                    # Devicetree-Bindings für die vendorten Treiber
├── CMakeLists.txt / Kconfig         # Modul-Einstiegspunkte für drivers/ (s. zephyr/module.yml)
├── build.yaml                       # CI-Build-Matrix (ein Eintrag pro Board-Hälfte)
├── zephyr/module.yml                # board_root + dts_root + cmake/kconfig für dieses Modul
├── README.md
├── ADDING_A_BOARD.md                # Kurz-Checkliste (Englisch)
└── docs/
    ├── MASTER_LAYOUT.md             # konzeptioneller Überblick (EN)
    ├── REFERENCE.en.md              # vollständige Referenz (EN)
    └── REFERENCE.de.md              # DIESE DATEI — vollständige Referenz (DE)
```

Die dünne Keymap jedes Boards bindet die geteilten Dateien in dieser Reihenfolge ein:

```c
#include <behaviors.dtsi>              // ZMK-System-Behaviors (&kp, &lt, &mt, ...)
#include <dt-bindings/zmk/keys.h>      // Key-Codes (Q, SPACE, C_MUTE, ...)
#include <dt-bindings/zmk/bt.h>        // BT_SEL, BT_CLR, ...
#include <dt-bindings/zmk/outputs.h>   // OUT_USB, OUT_BLE, ...

#include "geometry/geom_<board>.h"     // POS_* + KEYMAP_LAYER für dieses Board

#include "shared/keys_de.h"            // DE_*-Aliase
#include "shared/settings.dtsi"        // caps_word continue-list
#include "shared/behaviors.dtsi"       // HomeRowMods + cadet
#include "shared/macros.dtsi"          // MCR_*
#include "shared/combos.dtsi"          // Combos (brauchen POS_* aus dem geom-Header)
#include "shared/core_blocks.dtsi"     // CORE_*-Fragmente + WEAVE_*/RSVD_*
```

> Die Reihenfolge ist wichtig: Der Geometrie-Header muss vor `combos.dtsi`/
> `behaviors.dtsi` stehen (die dessen `POS_*` referenzieren), und ein Board, das eine
> AddOn-Taste **überschreibt**, muss den Override in seinem geom-Header *vor* dem
> Einbinden des AddOns `#define`n.

---

## 3. Koordinatensystem: der logische Core

Jedes Board bildet seine physischen Tasten auf dieselben **logischen Core**-Namen ab.
Lies sie als `POS_<Hand><Reihe><Fingerabstand-zur-Mitte>`:

- **Hand**: `L` (links) oder `R` (rechts)
- **Reihe**: `T` (oben/top), `M` (Mitte/home), `B` (unten/bottom)
- **Index 0..4**: Abstand vom inneren Handrand nach außen — `0` = innen (innere Spalte
  des Zeigefingers), `4` = Pinky-Spalte.

```
 LT0 LT1 LT2 LT3 LT4 | RT4 RT3 RT2 RT1 RT0     obere Reihe
 LM0 LM1 LM2 LM3 LM4 | RM4 RM3 RM2 RM1 RM0     Home-Reihe
 LB0 LB1 LB2 LB3 LB4 | RB4 RB3 RB2 RB1 RB0     untere Reihe
             LH0 LH1 | RH1 RH0                 innere 2 Daumen / Hand
```

Diese 34 Namen sind der Vertrag zwischen geteiltem Inhalt und jedem Board. `combos.dtsi`
und `behaviors.dtsi` referenzieren **ausschließlich** diese Symbole, sodass niemals eine
rohe physische Positionsnummer in `config/shared/` auftaucht. Der geom-Header eines
Boards bindet jeden Namen an seine eigene physische Nummer.

Boards mit mehr Tasten fügen **zusätzliche** Positionssymbole hinzu (dokumentiert in §4.1).

---

## 4. Variablen- & Makro-Katalog

Dieser Abschnitt listet und dokumentiert **jedes** geteilte Symbol.

### 4.1 Positions-Symbole `POS_*`

Definiert im Geometrie-Header eines Boards (oder im geteilten `core_3x5.h`). Es sind
schlichte Ganzzahlen = der physische Tasten-Positionsindex des Boards (wie ZMK die
Tasten in seinem `matrix_transform` nummeriert).

**Core (alle 34 verpflichtend — jedes Board definiert diese):**

| Symbol | Bedeutung | Symbol | Bedeutung |
|---|---|---|---|
| `POS_LT0..LT4` | links oben, innen→Pinky | `POS_RT4..RT0` | rechts oben, Pinky→innen |
| `POS_LM0..LM4` | links Home, innen→Pinky | `POS_RM4..RM0` | rechts Home, Pinky→innen |
| `POS_LB0..LB4` | links unten, innen→Pinky | `POS_RB4..RB0` | rechts unten, Pinky→innen |
| `POS_LH0, POS_LH1` | linke innere 2 Daumen | `POS_RH1, POS_RH0` | rechte innere 2 Daumen |

**Optionale Zusatztasten (nur auf Boards, die sie physisch haben):**

| Symbol | Bedeutung | Auf |
|---|---|---|
| `POS_LOT / POS_ROT` | äußere Pinky-Spalte, obere Reihe (L/R) | Corne, Cornholius, Splaytoraid40 |
| `POS_LOH / POS_ROH` | äußere Pinky-Spalte, Home-Reihe (L/R) | Corne, Cornholius, Splaytoraid40 |
| `POS_LOB / POS_ROB` | äußere Pinky-Spalte, untere Reihe (L/R) | Corne, Cornholius, TOTEM |
| `POS_LHX / POS_RHX` | 3. (äußerer) Daumen (L/R) | jedes Board mit 3. Daumen (inkl. TOTEM) |
| `POS_CENTER` | eine einzelne Mitteltaste (Encoder-Druck) | Le Chiffre, Splaytoraid |

**Geteilter Helfer `core_3x5.h`** — jedes Board, dessen Alphablock lückenlos `0..29`
(zeilenweise) nummeriert ist, `#include`t dies, um alle 30 Alpha-`POS_*` in einer Zeile
zu bekommen, statt sie zu wiederholen. Der Einbindende ergänzt dann seine eigenen
Daumen-/Zusatzpositionen. Indirekt von jedem Board genutzt, das über `geom_3x5_2.h` /
`geom_3x5_3.h` geht.

### 4.2 Core-Inhaltsfragmente `CORE_*`

Definiert in `config/shared/core_blocks.dtsi`. Jedes ist ein objektartiges Makro, das
sich zu einer **leerzeichengetrennten** Liste von ZMK-Bindings (keine Kommata)
expandiert. Benennung: `CORE_<layer>_<reihe>_<hand>`.

| Fragment (× 5 Layer) | Expandiert zu | Anzahl |
|---|---|---|
| `CORE_<L>_top_L` / `CORE_<L>_top_R` | obere Reihe (Alpha), links / rechts | je 5 |
| `CORE_<L>_home_L` / `CORE_<L>_home_R` | Home-Reihe (mit HomeRowMods auf base) | je 5 |
| `CORE_<L>_bot_L` / `CORE_<L>_bot_R` | untere Reihe (Alpha) | je 5 |
| `CORE_<L>_thumb_L` / `CORE_<L>_thumb_R` | innere 2 Daumen, links / rechts | je 2 |

wobei `<L> ∈ { base, nav, num, fun, pad }` — es gibt also **8 Fragmente × 5 Layer = 40**
`CORE_*`-Makros. Jedes Reihenfragment liefert 5 Bindings, jedes Daumenfragment 2, ein
Layer-Core also `5·6 + 2·2 = 34` Bindings.

Der **Inhalt** der base-Ebene (das kanonische Colemak-DH, DE-Host) ist:

```
oben    Q  W  F  P  B   |  J  L  U  Y  ;
home    A  R  S  T  G   |  M  N  E  I  O      (Home-Tasten sind HomeRowMods, siehe §4.8)
unten   Z  X  C  D  V   |  K  H  ,  .  /
Daumen  [C/S/A·Esc] [nav·Ret] | [num·Spc] [C/S/A·Del]
```

Die anderen vier Layer (`nav`, `num`, `fun`, `pad`) verwenden dieselben 40
Fragmentnamen mit anderen Bindings; die genauen Bindings stehen in `core_blocks.dtsi`
(die Single Source) und sind in §5 zusammengefasst. **Um eine Core-Taste zu ändern,
bearbeite genau dieses eine Fragment** — niemals eine Board-Keymap.

### 4.3 Weave-Helfer `WEAVE_*` und `RSVD_*`

Ebenfalls in `core_blocks.dtsi`:

| Makro | Expandiert zu | Verwendung |
|---|---|---|
| `WEAVE_ALPHA_3x5(L)` | die 30 Alpha-Bindings (oben,home,unten; L dann R je Reihe), **keine Daumen** | Boards, die eine eigene Daumenreihe hinter die Alphas setzen |
| `WEAVE_CORE(L)` | `WEAVE_ALPHA_3x5(L)` + innere 2 Daumen/Hand = 34 Bindings | reine `3×5+2`-Boards (`KEYMAP_LAYER(L) = WEAVE_CORE(L)`) |
| `RSVD_base` | `&none` | physisch vorhandene, aber unbelegte Taste, base-Ebene |
| `RSVD_nav/num/fun/pad` | `&trans` | dieselbe Taste auf höheren Layern (fällt durch) |

`RSVD_<L>` ist der „reservierte/inaktive“ Platzhalter: Webe `RSVD_##L` an eine Position,
die das Board hat, deren Funktion aber noch nicht feststeht — sie tut auf base nichts
und ist darüber transparent, ist jedoch verdrahtet und bereit, später durch echten
Inhalt ersetzt zu werden.

### 4.4 Das Weave-Makro `KEYMAP_LAYER`

Jeder Geometrie-Header definiert genau eines:

```c
#define KEYMAP_LAYER(L) <Fragmente, in der physischen Reihenfolge dieses Boards>
```

Die dünne Keymap ruft es einmal pro Layer auf: `bindings = <KEYMAP_LAYER(base)>;` usw.
Der Präprozessor fügt den Layer-Namen in jedes `CORE_##L##_*` / `OUTER_##L##_*` /
`THUMB_O_##L##_*`-Fragment ein und expandiert diese, **bevor** das Devicetree das
`<...>`-Array sieht. Da jedes Fragment leerzeichengetrennt ist, ist das Ergebnis eine
flache Binding-Liste, egal aus wie vielen Teilen sie zusammengesetzt wurde. Die Anzahl
der ausgegebenen Bindings **muss der physischen Tastenzahl des Boards entsprechen** (das
prüft §7).

### 4.5 AddOn: äußere Pinky-Spalte `OUTER_*`

`config/shared/addons/outer_col.h` — eine zusätzliche Spalte je Hand auf den drei
Alphareihen (6 Tasten). Fragmentname: `OUTER_<layer>_<reihe>_<hand>` (`reihe ∈
{top,home,bot}`, `hand ∈ {L,R}`) → **6 Fragmente × 5 Layer = 30** `OUTER_*`-Makros.

Standardinhalt:

| | links | rechts |
|---|---|---|
| oben (base) | `&kp ESC` | `&kp SQT` (`'`) |
| home (base) | `&kp MINUS` (`-`) | `&kp BSLH` (`\`) |
| unten (base) | `&cadet LSHFT LPAR` | `&cadet RSHFT RPAR` |
| alle höheren Layer | `&trans` | `&trans` |

`&cadet …` ist Space-Cadet-Shift (§4.8): Halten = Shift, Tippen = eine Klammer `(`/`)`.

**Jedes Fragment ist `#ifndef`-geschützt**, sodass ein Board eine einzelne Position
überschreibt, indem es sie *vor* dem `#include` des AddOns `#define`t:

```c
#define OUTER_base_top_L &kp TAB   /* dieses Board: Tab statt des Standard-Esc */
#include "../shared/addons/outer_col.h"   /* füllt die übrigen 29 Fragmente */
```

Benötigt das `cadet`-Behavior aus `behaviors.dtsi` (von jeder Keymap ohnehin eingebunden).

### 4.6 AddOn: 3. Daumen `THUMB_O_*`

`config/shared/addons/thumb_outer.h` — der 3. (äußere) Daumen je Hand. Fragmentname:
`THUMB_O_<layer>_<hand>` (`hand ∈ {L,R}`) → **2 Fragmente × 5 Layer = 10** Makros.

Standardinhalt: **ungenutzt** — `&none` auf base, `&trans` auf den höheren Layern.
Derselbe `#ifndef`-Override-Mechanismus wie bei `OUTER_*`: Um diesem Daumen auf *einem*
Board echten Inhalt zu geben, `#define THUMB_O_base_L …` vor dem Include; um ihn *jedem*
Board mit 3. Daumen zu geben, `thumb_outer.h` direkt bearbeiten.

### 4.7 Board-lokale Weave-Variablen

Manche Boards haben Tasten, die kein anderes Board teilt; die leben im eigenen
geom-Header des Boards (nicht in `shared/`). Sie folgen der Konvention `<NAME>_<layer>`,
damit `KEYMAP_LAYER` den Layer einsetzen kann. Aktuelle Beispiele:

| Variable | Board | Bedeutung |
|---|---|---|
| `CHIFF_CENTER_<layer>` | Le Chiffre | die Mitteltaste (`&kp C_MUTE` auf base, `&trans` darüber) |
| `SPLY_CENTER_<layer>` | Splaytoraid 40/36 | Encoder-Druck-Mitteltaste (`&kp C_MUTE` / `&trans`) |
| `C_R3_<layer>_L/R` | Cornholius | die äußeren Modifier-Tripel der 4. Reihe (Ctrl/Win/Alt · AltGr/Win/Ctrl) |
| `C_R3_TRANS` | Cornholius | Kürzel `&trans &trans &trans` für diese Reihe auf höheren Layern |

Wenn du ein Board mit einer einzigartigen Taste hinzufügst, lege eine solche Variable in
seinem geom-Header an — dieser Benennung folgend, damit `KEYMAP_LAYER` sie je Layer
einweben kann.

### 4.8 Behaviors (HomeRowMods + Cadet-Shift)

`config/shared/behaviors.dtsi`. Alle sind `zmk,behavior-hold-tap` mit `#binding-cells =
<2>` (aufgerufen als `&<name> <hold-param> <tap-param>`). Parameter 1:1 aus der
kanonischen TOTEM-Keymap.

| Label | Node-Name | Halten / Tippen | tapping-term-ms | flavor | positionell? |
|---|---|---|---|---|---|
| `&hmg` | `homerow_mod_gui` | GUI (beim Halten) / Buchstabe | 500 | balanced | nein |
| `&hmla` | `homerow_mod_lalt` | Linke Alt / Buchstabe | 400 | balanced | nein |
| `&hmls` | `homerow_mod_lshift` | Linke Shift / Buchstabe | 150 | balanced | ja (rechter Core) |
| `&hmlc` | `homerow_mod_lctrl` | Linke Ctrl / Buchstabe | 120 | balanced | ja (rechter Core) |
| `&hmrc` | `homerow_mod_rctrl` | Rechte Ctrl / Buchstabe | 120 | balanced | ja (linker Core) |
| `&hmrs` | `homerow_mod_rshift` | Rechte Shift / Buchstabe | 180 | balanced | ja (linker Core) |
| `&hmra` | `homerow_mod_ralt` | Rechte Alt / Buchstabe | 800 | tap-preferred | nein |
| `&cadet` | `cadet_shift` | Shift / Klammer | 200 | balanced | nein |

Für alle gleich: `quick-tap-ms = <0>`. `hmlc`/`hmrc` setzen zusätzlich
`hold-while-undecided`.

**Positionelle Hold-Taps** (`hmlc/hmrc/hmls/hmrs`) beschränken das Halten auf den Fall,
dass eine Taste auf dem Core der **Gegenhand** gedrückt ist, mittels
`hold-trigger-key-positions`. Zwei geteilte Listen bauen diese aus `POS_*`:

- `TRIGGER_RIGHT` = die 15 Core-Positionen der rechten Hand (`RT4..RT0 RM4..RM0
  RB4..RB0`) — genutzt von den **linken** HRMs.
- `TRIGGER_LEFT` = die 15 Core-Positionen der linken Hand — genutzt von den **rechten**
  HRMs.

Da diese aus Symbolen aufgebaut sind, sind sie board-unabhängig. Auf base ist die
Home-Reihe: `&hmg LGUI A` `&hmla LALT R` `&hmls LSHFT S` `&hmlc LCTRL T` `G` | `M`
`&hmrc RCTRL N` `&hmrs RSHFT E` `&hmla LALT I` `&hmg RGUI O`, dazu `&hmra RALT X` und
`&hmra RALT DOT` in der unteren Reihe.

### 4.9 Makros `MCR_*`

`config/shared/macros.dtsi`, alle `zmk,behavior-macro` (`#binding-cells = <0>`,
aufgerufen als `&MCR_…`). Auf ein **deutsches Host-Layout** abgestimmt.

| Makro | Erzeugt | Sequenz |
|---|---|---|
| `&MCR_DQT` | `"` dann Leerzeichen | LSHFT drücken · `'` tippen · LSHFT loslassen · Space tippen |
| `&MCR_SCH` | `sch` (nur das S groß) | S tippen · beide Shifts loslassen · C, H tippen |
| `&MCR_SQT` | `'` dann Leerzeichen | `'` tippen · Space tippen |
| `&MCR_TILDE` | `~` dann Leerzeichen | RSHFT drücken · Grave tippen · RSHFT loslassen · Space tippen |
| `&MCR_DEGREE` | `°` | RALT+LSHFT drücken · `;` tippen · beide loslassen |

### 4.10 Deutsche Aliase `DE_*`

`config/shared/keys_de.h`. Komfort-Aliase zum Verfassen; die kanonischen Layer erzeugen
diese Zeichen über Combos statt über die Aliase. **Der Host muss auf ein deutsches
(DE-)Tastaturlayout eingestellt sein** — die Umlaute entstehen als AltGr-Kombinationen
(`RA = RIGHT_ALT`).

| Alias | Wert | Zeichen |
|---|---|---|
| `DE_AE` | `RA(Q)` | ä |
| `DE_OE` | `RA(P)` | ö |
| `DE_UE` | `RA(Y)` | ü |
| `DE_SS` | `RA(S)` | ß |
| `DE_EUR` | `RA(N5)` | € |

### 4.11 Combos

`config/shared/combos.dtsi` — 37 Combos, alle auf Core-Positionen (daher
board-unabhängig; noch keine Guards nötig). `layers = <0..4>` entsprechen `base nav num
fun pad`.

| Name | Ausgabe | Positionen | Layer |
|---|---|---|---|
| `Cmb_UmlA` | ä (`RA(Q)`) | LT0 LM0 | base |
| `Cmb_UmlO` | ö (`RA(P)`) | RT0 RM0 | base |
| `Cmb_UmlU` | ü (`RA(Y)`) | RT2 RM2 | base |
| `Cmb_UmlS` | ß (`RA(S)`) | LT2 LM2 | base |
| `Cmb_EUR` | € (`RA(N5)`) | RM2 RB2 | base |
| `Cmb_ESC` | Esc | LT0 LT1 | alle |
| `Cmb_TAB` | Tab | LB2 LB3 | base,nav,num,pad |
| `Cmb_CLCKs` | Caps Lock | LM0 LM1 LM2 LM3 | nav |
| `Cmb_CapsWord` | `&caps_word` | LM2 RM2 | base |
| `Cmb_BSPC` | Backspace | RB3 RB2 | base,num,pad |
| `Cmb_DEL` | Entf | RB2 RB1 | base,num,pad |
| `Cmb_CtrlBSPC` | Strg+Backspace | RB3 RB2 RB1 | base,num |
| `Cmb_CtrlDEL` | Strg+Entf | RB2 RB1 RB0 | base,num |
| `Cmb_WordLeft` | Strg+← | LM1 LM2 | nav |
| `Cmb_WordRight` | Strg+→ | LM2 LM3 | nav |
| `Cmb_Cut` | Strg+X | LB0 LB1 | base |
| `Cmb_Copy` | Strg+C | LB1 LB2 | base |
| `Cmb_Paste` | Strg+V | LB0 LB1 LB2 | base |
| `Cmb_Reset` | `&bootloader` | LM1 LM2 LM3 | pad |
| `Cmb_UNDO` | Strg+Z | LT2 LT3 | base |
| `Cmb_REDO` | Strg+Y | RT3 RT2 | base |
| `Cmb_ALL` | Strg+A | LM0 LM1 | base |
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

### 4.12 Globale Einstellungen

`config/shared/settings.dtsi`:

```c
&caps_word { continue-list = <UNDERSCORE MINUS BSPC DEL>; };
```

Erweitert ZMKs Caps-Word, sodass es über `_`, `-`, Backspace und Entf aktiv bleibt.

### 4.13 Kconfig-(`.conf`-)Variablen

Je Board nach Funktyp gewählt über `-DEXTRA_CONF_FILE=…` in `build.yaml`. Eine
board-spezifische `config/<board>.conf` (falls vorhanden) wird für board-eigene Extras
darüber gemischt.

**`config/shared_ble.conf`** — kabellose / BLE-Boards (nRF: nice!nano & Klone, xiao_ble):

| Variable | Wert | Bedeutung |
|---|---|---|
| `CONFIG_BT` | `y` | Bluetooth aktivieren |
| `CONFIG_BT_CTLR_TX_PWR_PLUS_8` | `y` | +8 dBm Sendeleistung |
| `CONFIG_ZMK_SLEEP` | `y` | Deep-Sleep-Unterstützung |
| `CONFIG_ZMK_IDLE_SLEEP_TIMEOUT` | `600000` | Sleep nach 10 min Leerlauf (ms) |
| `CONFIG_BT_MAX_CONN` | `6` | max. gleichzeitige Verbindungen |
| `CONFIG_BT_MAX_PAIRED` | `6` | max. gekoppelte Profile |
| `CONFIG_ZMK_KSCAN_DEBOUNCE_PRESS_MS` | `1` | „eager“ Press-Entprellung |
| `CONFIG_ZMK_KSCAN_DEBOUNCE_RELEASE_MS` | `5` | Release-Entprellung |
| `CONFIG_ZMK_COMBO_MAX_PRESSED_COMBOS` | `8` | Combo-Engine-Limits… |
| `CONFIG_ZMK_COMBO_MAX_COMBOS_PER_KEY` | `16` | …erhöht, weil die geteilte Konfig… |
| `CONFIG_ZMK_COMBO_MAX_KEYS_PER_COMBO` | `5` | …viele überlappende Combos nutzt |
| `CONFIG_ZMK_BLE_EXPERIMENTAL_FEATURES` | `y` | experimenteller BLE-Stack |
| `CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING` | `y` | Peripherie-Akkustand (Split) |

**`config/shared_usb.conf`** — kabelgebundene USB-only-Boards (RP2040-„Pro-Micro“-Klone).
Gleiche Entprellung + Combo-Limits, **kein** Bluetooth:

| Variable | Wert | Bedeutung |
|---|---|---|
| `CONFIG_ZMK_USB` | `y` | USB-HID-Ausgabe |
| `CONFIG_ZMK_KSCAN_DEBOUNCE_PRESS_MS` | `1` | „eager“ Press-Entprellung |
| `CONFIG_ZMK_KSCAN_DEBOUNCE_RELEASE_MS` | `5` | Release-Entprellung |
| `CONFIG_ZMK_COMBO_MAX_PRESSED_COMBOS` | `8` | Combo-Limits (wie oben) |
| `CONFIG_ZMK_COMBO_MAX_COMBOS_PER_KEY` | `16` | |
| `CONFIG_ZMK_COMBO_MAX_KEYS_PER_COMBO` | `5` | |

> Setze nie `CONFIG_BT…` in die Konfig eines USB-Boards — diese Boards haben keinen
> Funk, und der Build schlägt fehl. Encoder-`CONFIG_EC11…` gehört in die **eigene**
> `<shield>.conf` des Shields, nicht in die geteilte Konfig. `settings_reset`-Builds
> lassen die geteilte Konfig bewusst weg.

### 4.14 Encoder (`sensor-bindings`)

Dreh-Encoder sind orthogonal zum Tastenraster — sie sind **Sensoren**, keine
Tastenpositionen, tauchen also nie in `KEYMAP_LAYER` auf. Ein Board mit Encodern fügt
jedem Layer-Node in seiner dünnen Keymap eine `sensor-bindings`-Eigenschaft hinzu, in
derselben Reihenfolge wie die `sensors = <…>`-Liste des Shields. Das übliche Behavior
ist `&inc_dec_kp <ccw> <cw>`.

Beispiel (KLOTZ, zwei Encoder — Lautstärke, Cursor):

```c
#define ENCODERS  &inc_dec_kp C_VOL_DN C_VOL_UP   &inc_dec_kp LEFT RIGHT
...
base { bindings = <KEYMAP_LAYER(base)>; sensor-bindings = <ENCODERS>; };
```

Die EC11-Hardware wird in der eigenen `<shield>.conf` des Shields aktiviert
(`CONFIG_EC11=y`, `CONFIG_EC11_TRIGGER_GLOBAL_THREAD=y`).

---

## 5. Die fünf Layer

Layer-Indizes und wie jeder erreicht wird (alle Zugriffstasten liegen auf
Core-Positionen, die jedes Board hat, damit jedes Board jeden Layer erreicht):

| # | Name | Erreicht durch | Zweck |
|---|---|---|---|
| 0 | `base` | Standard | Colemak-DH-Buchstaben, HomeRowMods, Daumen-Layer-Taps |
| 1 | `nav` | linken inneren Daumen halten (`&lt 1 RET`) | Pfeile, Pos1/Ende/Bild↑/Bild↓, BT-Profilwahl, Medien |
| 2 | `num` | rechten inneren Daumen halten (`&lt 2 SPACE`) | Zahlen + Symbole (geshiftete Zahlenreihe, Klammern) |
| 3 | `fun` | `&mo 3` (rechter Daumen auf `nav`) | F1–F12 + Modifier |
| 4 | `pad` | `&tog 4` (linker Daumen auf `num`); `&to 0` zurück | Ziffernblock; `&bootloader`-Combo liegt hier |

Die genauen Bindings aller fünf Layer stehen in `config/shared/core_blocks.dtsi` (die
Single Source). Zusammenfassung der Nicht-base-Layer:

- **nav**: oben = Esc/Druck/↑/Einfg/Bild↑ · BT_SEL 0–4; home = Pos1/←/↓/→/Ende ·
  App/Modifier; unten = Rollen/Pause/BSPC/Entf/Bild↓ · BT vor/zurück/löschen.
- **num**: oben = geshiftete `1..5` / `6..0`; home = `1..5` / `6..0` mit HRMs; unten =
  `°`, `'`, BSPC, Entf, `[` / `]`, `,`, `.`.
- **fun**: oben = F1–F5 / F6–F10; home = GUI/Alt/Shift/Ctrl/F11 / F12/…; auf den Daumen
  liegen `&tog 4` und `&mo 3`.
- **pad**: die rechte Hand ist ein Ziffernblock (`7 8 9 - / * 4 5 6 + . 1 2 3 ,`); `&to
  0` oben links kehrt zu base zurück.

---

## 6. Schritt für Schritt: ein Board hinzufügen (vollständiges Beispiel)

Wir fügen ein **fiktives** Board hinzu, um den gesamten Ablauf zu zeigen — bewusst
*größer* als `3×5+2` gewählt und mit **Override**.

### Das fiktive Board: **„Kolibri“**

- Kabellos (nice!nano, nRF52840).
- **3×6 + 3 Daumen/Hand = 42 Tasten** (eine äußere Pinky-Spalte auf allen drei Reihen
  plus ein 3./äußerer Daumen je Hand) — ein Superset, kann also `geom_3x5_2/3` nicht
  wiederverwenden.
- Ein Dreh-**Encoder** auf der linken Hälfte → Lautstärke.
- Zwei **Overrides** gegenüber den geteilten Standards:
  1. Die obere linke Taste der äußeren Spalte soll **Tab** sein, nicht das Standard-**Esc**;
  2. die 3. Daumen sollen **GUI**-Tasten sein, nicht das Standard-`&none`.

Physische Matrix (lückenlos `0..41`, zeilenweise, wie ZMK sie nummeriert):

```
 0  1  2  3  4  5 |  6  7  8  9 10 11     Reihe 0 (0,11 = äußere Pinky-Spalte)
12 13 14 15 16 17 | 18 19 20 21 22 23     Reihe 1 (12,23 = äußere Pinky-Spalte)
24 25 26 27 28 29 | 30 31 32 33 34 35     Reihe 2 (24,35 = äußere Pinky-Spalte)
      36 37 38    | 39 40 41              Daumen (36,41 = 3./äußerer Daumen)
```

### Schritt 1 — Hardwaredefinition bereitstellen

Vendore das Shield unter `boards/shields/kolibri/` (oder referenziere ein Upstream-Modul
in `config/west.yml`, falls ein gutes existiert). Kopiere **nur** die Hardwaredateien —
`kolibri.overlay` / `kolibri.dtsi` / etwaige `*-layouts.dtsi`, `Kconfig.shield`,
`Kconfig.defconfig` und für einen Split `kolibri.zmk.yml`. Kopiere **nicht** die
`keymap`, `keys_*.h` oder Funk-`.conf` des Quell-Repos — die kommen aus `config/shared/`.

`Kconfig.shield` muss den Guard nach dem Shield benennen:

```kconfig
config SHIELD_KOLIBRI
    def_bool $(shields_list_contains,kolibri)
```

### Schritt 2 — Geometrie-Adapter `config/geometry/geom_kolibri.h` schreiben

Da Kolibri ein Superset ist, bekommt es einen eigenen Header. Beachte: die **Overrides
werden vor den AddOn-Includes `#define`t**.

```c
/*
 * geom_kolibri.h — fiktives 3x6 + 3-Daumen/Hand-Board (42 Tasten), lückenlos 0..41.
 * Äußere Pinky-Spalte auf allen 3 Reihen (addons/outer_col.h) + ein 3. Daumen je Hand
 * (addons/thumb_outer.h). Zwei Overrides: Tab oben links, GUI auf den 3. Daumen.
 */
#pragma once

/* --- Core-Positionen (logischer Core -> physische Kolibri-Nummern) --- */
/* oben   */ #define POS_LT0 1
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
/* unten  */ #define POS_LB0 25
             #define POS_LB1 26
             #define POS_LB2 27
             #define POS_LB3 28
             #define POS_LB4 29
             #define POS_RB4 30
             #define POS_RB3 31
             #define POS_RB2 32
             #define POS_RB1 33
             #define POS_RB0 34
/* innere 2 Daumen/Hand */
             #define POS_LH0 37
             #define POS_LH1 38
             #define POS_RH1 39
             #define POS_RH0 40

/* --- Zusatztasten, die Kolibri physisch hat (Symbole, damit Combos/HRM sie treffen könnten) --- */
#define POS_LOT 0    /* äußere Spalte, oben,  links  */
#define POS_LOH 12   /* äußere Spalte, home,  links  */
#define POS_LOB 24   /* äußere Spalte, unten, links  */
#define POS_ROT 11   /* äußere Spalte, oben,  rechts */
#define POS_ROH 23   /* äußere Spalte, home,  rechts */
#define POS_ROB 35   /* äußere Spalte, unten, rechts */
#define POS_LHX 36   /* 3. (äußerer) Daumen, links   */
#define POS_RHX 41   /* 3. (äußerer) Daumen, rechts  */

/* --- OVERRIDES: müssen VOR den AddOn-Includes stehen (die Guards lassen unsere gewinnen) --- */
#define OUTER_base_top_L  &kp TAB    /* Kolibri: Tab (Standard war Esc) */
#define THUMB_O_base_L    &kp LGUI   /* Kolibri: GUI auf den 3. Daumen (Standard &none) */
#define THUMB_O_base_R    &kp RGUI

#include "../shared/addons/outer_col.h"    /* füllt den Rest der äußeren Spalte */
#include "../shared/addons/thumb_outer.h"  /* füllt den Rest der 3.-Daumen-Fragmente */

/* --- Core + AddOns in Kolibris physische Reihenfolge weben --- */
#define KEYMAP_LAYER(L) \
    OUTER_##L##_top_L   CORE_##L##_top_L   CORE_##L##_top_R   OUTER_##L##_top_R  \
    OUTER_##L##_home_L  CORE_##L##_home_L  CORE_##L##_home_R  OUTER_##L##_home_R \
    OUTER_##L##_bot_L   CORE_##L##_bot_L   CORE_##L##_bot_R   OUTER_##L##_bot_R  \
    THUMB_O_##L##_L CORE_##L##_thumb_L CORE_##L##_thumb_R THUMB_O_##L##_R
```

Zähle das Weave eines Layers: `6 (äußere) + 30 (Alpha) + 4 (Core-Daumen) + 2 (3. Daumen)
= 42` — passt zur Matrix. Gut.

> Warum die Overrides vor den Includes stehen: Das AddOn umhüllt jeden Standard mit
> `#ifndef`. Wenn du `OUTER_base_top_L` zuerst `#define`st, überspringt das `#ifndef` des
> AddOns seinen eigenen Standard und behält deinen. Definierst du es *nach* dem Include,
> gibt es einen Redefinitionsfehler. Der Override dieses einen Boards betrifft **kein**
> anderes Board.

### Schritt 3 — die dünne Keymap `config/kolibri.keymap` schreiben

```c
/*
 * Kolibri (fiktiv) — dünne Keymap. 3x6 + 3 Daumen/Hand auf nice_nano, ein linker
 * Encoder (Lautstärke). Aller Inhalt liegt in config/shared/.
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

/* ein Encoder auf der linken Hälfte -> Lautstärke runter/rauf */
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

(Hat das Board keinen Encoder, lass die `sensor-bindings` und das `ENCODERS`-Define weg.)

### Schritt 4 — board-eigene conf nur bei Bedarf

Kolibri braucht das EC11 des Encoders — aber das liegt in der **eigenen**
`boards/shields/kolibri/kolibri.conf` des Shields:

```kconfig
CONFIG_EC11=y
CONFIG_EC11_TRIGGER_GLOBAL_THREAD=y
```

Eine `config/kolibri.conf` ist nicht nötig, außer das Board hat etwas Zusätzliches (z. B.
ein Display). Die Funk-Einstellungen kommen im nächsten Schritt aus `shared_ble.conf`.

### Schritt 5 — zu `build.yaml` hinzufügen

```yaml
  # --- Kolibri (fiktives 3x6+3-BLE-Demo; vendortes Shield boards/shields/kolibri) ---
  - board: nice_nano//zmk
    shield: kolibri
    cmake-args: -DEXTRA_CONF_FILE=../../config/shared_ble.conf
```

Für ein **Split**-Board fügst du Einträge `kolibri_left` und `kolibri_right` hinzu; für
ein **kabelgebundenes RP2040**-Board nimm `sparkfun_pro_micro_rp2040//zmk` +
`shared_usb.conf`; ergänze einen `settings_reset`-Eintrag für den Controller, wenn du ein
Reset-Image möchtest.

### Schritt 6 — verifizieren (vor dem Push)

Kopiere `scratchpad/verify_endgame.py` nach `verify_kolibri.py`, ändere die Ziel-Keymap
auf `kolibri.keymap` und die erwartete Anzahl auf `42`, führe dann das TOTEM-Gate plus
die neue Prüfung aus:

```
python3 verify.py            # TOTEM muss byte-identisch bleiben (geteilter Inhalt unangetastet)
python3 verify_kolibri.py    # kolibri muss auf allen 5 Layern exakt 42 Bindings ergeben
```

Dann committen und pushen; CI (`build.yaml`) baut das Board gegen `zmk main`.

### Was du **nicht** angefasst hast

Keine geteilte Datei wurde geändert. Kolibri hinzuzufügen berührte nur: sein vendortes
Shield, sein `geom_kolibri.h`, seine dünne `kolibri.keymap` und einen `build.yaml`-Eintrag.
Jedes andere Board bleibt unberührt — und wenn du später eine Core-Taste in
`core_blocks.dtsi` änderst, erbt Kolibri sie automatisch.

---

## 7. Verifikation

Korrektheit wird durch Präprozessieren bewiesen, nicht nur durch „es baut“:

- **`scratchpad/verify.py`** lässt den C-Präprozessor über die *originale*
  `zmk-config-totem`-Keymap und unsere `totem.keymap` laufen und vergleicht die
  expandierten `bindings` jedes Layers sowie jedes Combo. TOTEMs **34-Tasten-Core muss
  byte-identisch** bleiben — das Regressions-Gate, das beweist, dass eine Änderung am
  geteilten Inhalt das kanonische Layout nicht verändert hat (5/5 Layer, alle 37 Combos
  identisch). TOTEMs vier äußere Tasten (die unteren Extra-Pinkys + die 3. Daumen) werden
  jetzt bewusst von den geteilten AddOns gesteuert und sind daher vom Byte-Vergleich
  ausgenommen.
- **Board-Binding-Anzahl** (`verify_cb34s.py`, `verify_endgame.py`, …): die Keymap eines
  Boards muss zu exakt seiner physischen Tastenzahl präprozessieren (34 für `3×5+2`, 36
  für `3×5+3`, 42 für das Kolibri-Beispiel). Das fängt ein falsch gewebtes
  `KEYMAP_LAYER` (falsche Reihenfolge, ausgelassene/doppelte Position) vor der CI ab. Die
  Skripte nutzen den von `verify.py` erzeugten bereinigten Baum wieder.
- **CI** (GitHub Actions, `build.yaml`) baut anschließend jedes Board + ein
  `settings_reset`-Image gegen `zmk main`.

---

## 8. Board-Hardware: Bezugsquellen

Die Hardware eines Boards (Shield-`matrix_transform`, kscan, Overlays; oder ein
Controller-Board) stammt aus einer von zwei Quellen:

- **Upstream-ZMK-Modul** — in `config/west.yml` referenziert; die CI holt es zur
  Build-Zeit. Bevorzugt, wenn ein Modul sauber gegen unser `zmk main` baut. Aktuelle
  Module: Urchin (`duckyb/urchin-zmk-module`), re-gret (`rschenk/zmk-keyboard-re-gret` @
  v0.4), forager (`carrefinho/forager-zmk-module`), delta-omega
  (`unspecworks/zmk-keyboard-delta-omega`), dazu das Display `nice-view-gem` und das
  `zmk-rgbled-widget` (eine harte Abhängigkeit von delta-omega, workspace-weit nötig).
- **Vendort** unter `boards/shields/<name>/` — eine lokale Kopie, wenn kein verteilbares
  Modul existiert oder ein Modul nicht gegen unsere Revision baut. Dieses eine
  Verzeichnis enthält sowohl reine Shields als auch vollständige HWv2-Controller-Boards
  (auf Zephyr 4.1 migriert: Cornholius, Le Chiffre BLE, Le Chiffre 36 STM32, Tipper TF)
  — Zephyr erkennt ein Board an seiner `board.yml`, unabhängig vom Pfad, daher können
  sich Boards und Shields den Ordner teilen. `zephyr/module.yml` setzt `board_root: .`,
  damit `boards/` im Suchpfad liegt.

### Vendorte Out-of-Tree-Treiber

Braucht ein Board einen Treiber, der nicht in `zmk main` ist, wird er unter `drivers/`
mit seinem Binding unter `dts/bindings/` vendort, und die `zephyr/module.yml` dieses
Repos registriert beides (`build.cmake` → Wurzel-`CMakeLists.txt`, `build.kconfig` →
Wurzel-`Kconfig`, `dts_root: .`). Ein Treiber wird **nur** kompiliert, wenn sein
`CONFIG_*` gesetzt ist, das Modul ist also für jedes Board ohne Opt-in inaktiv.
Aktuelles Beispiel: **`drivers/display/jd79653.c`** — der 1,54″-GoodDisplay-(JD79653-)
Epaper-Controller des Tipper TF, aus weteors ZMK-Fork (alte Zephyr-2.x-API) auf die
aktuelle Zephyr-Display/SPI/GPIO-API portiert (nach dem Vorbild von ZMKs internem
`il0323`-Treiber). Die `config/tipper_tf.conf` des Boards schaltet ihn mit
`CONFIG_JD79653=y` + `CONFIG_ZMK_DISPLAY=y` ein.

Deine alten Board-Repos `zmk-config-*` zu löschen ist sicher: die Shield-/Board-
Definitionen sind vendort (oder werden als Module geladen), und aller Inhalt liegt in
`config/shared/`.
