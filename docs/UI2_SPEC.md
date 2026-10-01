# X4 MultiHub UI 2.0 - implementation candidate

This patch is the graphical dashboard pass requested after the first successful
ESP32-S3/X4C physical flash.

## Hardware target stays unchanged

- XTEINK X4 Classic / X4C
- ESP32-S3
- 8 MB PSRAM
- 16 MB flash
- CrossPoint 1.6.5 base
- PlatformIO environment `x4c-gh_release`
- APPLICATION BIN only
- no automatic erase
- no partition-table replacement

## UI direction

The new main screen follows the supplied monochrome reference board:

- CrossPoint clock/battery/status band remains at the top
- large `X4 MultiHub` identity
- subtitle `Centrum czytania, organizacji i informacji`
- 3-column, high-contrast e-ink card grid
- large 32-38 px monochrome icons
- selected card inverts black/white
- no animations
- no decorative gradients
- buttons and touch share the same action map

Primary tiles:

1. Czytnik
2. Biblioteka
3. Field Manual
4. Planer
5. News Terminal
6. Pogoda
7. Rynki
8. Ustawienia

Bottom quick actions:

- Dashboard
- Diagnostyka
- Gateway

## Navigation

X4C front directional buttons move spatially across the grid.
Confirm opens a module. Back exits. Side/page navigation also walks the items
linearly, so the UI remains usable with either X4C button style.

## Existing functionality is preserved

The patch only replaces the MultiHub shell/navigation presentation and adds a
Gateway deep-link into the existing settings screen. It does not replace the
existing reader, planner, manuals, market, weather, news, dashboard, gateway,
cache, power, time or diagnostics logic.

The specialist screens continue using their proven FreeInk/CrossPoint list and
reader components in this candidate. Their data model and online/offline
behaviour are unchanged. Further visual specialization can be done after this
dashboard build is physically checked for spacing, font rendering and e-ink
ghosting on the real X4C.
