# X4 MultiHub v1.0 — functional specification

## Home

Main entries:

1. Reader
2. Field Manual
3. Daily Planner
4. Markets & Weather
5. Dashboard
6. Settings

## Reader

Target: preserve the stable CrossPoint reading experience and expose it from
the MultiHub navigation. EPUB/TXT and other formats remain governed by the
capabilities of the pinned CrossPoint base.

## Field Manual

Offline-first documents stored on microSD.

Planned:
- categories,
- text/manual browser,
- search,
- favorites,
- checklists,
- recently opened,
- import by copying files to SD.

## Daily Planner

Offline-first.

Planned:
- Today,
- tasks,
- priorities,
- notes,
- checklists,
- habits,
- simple calendar/day navigation,
- completed/archive.

## Markets & Weather

Online data only when Wi-Fi and gateway are reachable.

Finance:
- GPW / WAR catalogue,
- instruments from NewConnect when actually present in provider catalogue,
- Forex,
- crypto,
- dynamic search,
- add/remove favorites,
- custom ordering,
- latest data timestamp,
- explicit data mode/age when available.

Weather:
- user-selected location,
- current weather,
- selected short forecast fields,
- cache of last successful response.

## Dashboard

User-configurable cards:
- weather,
- selected FX,
- selected WAR instruments,
- selected crypto,
- planner summary,
- current book/progress.

No fixed list of stocks or coins.

## Settings

Planned:
- Wi-Fi via existing CrossPoint facilities,
- X4 Data Gateway URL,
- weather location,
- refresh interval,
- favorite instruments,
- units,
- cache management,
- SD diagnostics,
- About / build information.

## Safety / truthfulness

- UNKNOWN blocks operations that require the missing value.
- No automatic `erase_flash`.
- Application BIN is never labelled as a full-flash image.
- Missing market data is `DATA UNAVAILABLE`.
- A delayed/EOD quote is not labelled real-time unless the provider metadata
  supports that statement.
