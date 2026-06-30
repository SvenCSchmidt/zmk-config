/*
 * keys_de.h — German-layout helper aliases, shared by every board.
 *
 * The canonical TOTEM content produces German special characters via AltGr
 * (RA = RIGHT_ALT) on a host configured for the German (DE) keyboard layout,
 * e.g. RA(Q)=ä, RA(P)=ö, RA(Y)=ü, RA(S)=ß, RA(N5)=€. The umlaut combos in
 * combos.dtsi use those sequences inline.
 *
 * HOST LAYOUT ASSUMPTION: the firmware assumes the operating system is set to a
 * German (DE) keyboard layout. The aliases below are provided for convenience
 * when authoring new content; the canonical layers do not depend on them, so the
 * file is intentionally light.
 */

#pragma once

#define DE_AE  RA(Q)   /* ä */
#define DE_OE  RA(P)   /* ö */
#define DE_UE  RA(Y)   /* ü */
#define DE_SS  RA(S)   /* ß */
#define DE_EUR RA(N5)  /* € */
