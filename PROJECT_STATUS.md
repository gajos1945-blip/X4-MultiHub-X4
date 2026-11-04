# PROJECT_STATUS — X4 MultiHub

Ostatnia aktualizacja: 2026-11-04

## Stan
- Wersja: `1.7-dev` (na bazie zamrożonego `v1.0.0-rc1`), status `DEVELOPMENT_V1_7_HARDWARE_UNVERIFIED`.
- Software feature complete dla rc1: Reader, Field Manual, Daily Planner, Markets, Weather, Dashboard, cache/resilience, Settings.
- Po rc1: News Terminal RSS/Atom (1.1), Power Manager (1.2), Time/NTP/Today (1.3), Reader Dashboard (1.4),
  Windows Gateway (1.5), Gateway Access Control (1.6), Diagnostyka (1.7).
- Testy: 155 passed (`python -m pytest -q`, 2026-11-04).

## Decyzje
- 2026-11-04: projekt `CrossPoint-MAX-X4` zakończony; dalszy rozwój tylko w X4 MultiHub.
- Overlay + patcher zamiast forka CrossPoint — łatwa aktualizacja bazy, zmiany upstreamu w jednym miejscu.
- Fail-closed CI: brak BIN, jeśli flash/offset/partycje są UNKNOWN.
- Gateway HTTP w LAN, tylko token (bez TLS) — świadomie; nie wystawiać do Internetu.

## Otwarte
- Fizyczna walidacja na X4: boot, e-ink/przyciski, Wi-Fi, gateway z urządzenia, microSD — NOT RUN.

## Następne kroki
1. Test na fizycznym X4 wg `docs/RC_CHECKLIST.md`.
