# X4 MultiHub — instrukcje dla Claude

Firmware dla XTEINK X4 (ESP32-C3, e-ink) zbudowany jako **overlay na przypiętym CrossPoint Reader 1.6.0**
(commit `54337e6d73fc628f4ba523ddc89a743ca8c6e4c5`, env `gh_release`) + Pythonowy **X4 Data Gateway** w LAN.
Status: `1.7-dev`, `HARDWARE_UNVERIFIED`. Bieżący stan i kolejne kroki: `PROJECT_STATUS.md`.

## Struktura
- `.builder/overlay/src/multihub/` — cały kod C++ MultiHub (Activity = ekran, Store = stan na microSD, *GatewayClient = HTTP do gatewaya).
- `.builder/tools/patch_crosspoint.py` — wstrzykuje overlay do sklonowanego upstreamu (`replace_once` / `ensure_absent`). Zmiany w upstreamie TYLKO tędy.
- `.builder/build_ci.py` — klon upstreamu → patch → `pio run` → weryfikacja BIN (`verify_project`, `bin_inspector`, `release_guard`, `rc_audit`).
- `.builder/manifest.json` — wersja, status, lista `implemented`; testy sprawdzają jej zgodność.
- `.builder/tests/` — statyczne testy firmware/buildera; `tests/` — testy gatewaya.
- `gateway/multihub_gateway/` — serwer HTTP; providery: EODHD, Open-Meteo, RSS. `windows_app.py` + PyInstaller (`X4DataGateway.spec`).
- `sdcard/.x4-multihub/` — układ katalogów na karcie. `docs/` — ARCHITECTURE, SPEC_v1, ROADMAP, RC_CHECKLIST.

## Komendy
- Testy (całość, ~4 s): `python -m pytest -q`
- Build firmware: tylko GitHub Actions (`.github/workflows/build-x4-bin.yml`, push na `main`). Lokalnie wymaga PlatformIO (pioarduino 6.1.19) i dostępu do sieci.
- Gateway: `python gateway/server.py` (token opcjonalnie w `MULTIHUB_GATEWAY_TOKEN`).

## Zasady nienaruszalne
- NIGDY nie wpisuj na sztywno flash size, offsetów ani partycji — CI odczytuje je z upstreamu i blokuje build przy UNKNOWN (fail-closed).
- Workflow nie może wykonywać `erase_flash`, flashowania ani tworzyć merged/full-flash image.
- Żadnych kluczy API/tokenów w firmware ani w repo (repo jest publiczne). Klucze tylko w env/DPAPI po stronie gatewaya.
- Nie zmieniaj przypiętego commitu CrossPoint bez wyraźnej prośby.
- Nie oznaczaj niczego jako „hardware verified” — fizyczny test X4 nie został wykonany.
- Nowa funkcja = wpis w `manifest.json` + marker wymagany przez `release_guard` + test w `.builder/tests/`.
- Firmware to klient low-RAM: ciężka logika i normalizacja danych należą do gatewaya, nie do X4.

## Praca
- Po edycji `.py` hook (`.claude/hooks/post_edit_check.py`) uruchamia ruff (F) i pytest; błąd wraca do Claude.
- Dokumentacja bibliotek (PlatformIO, ESP-IDF/Arduino-ESP32, Python): używaj Context7 zamiast zgadywać API.
- Do nawigacji po C++ preferuj Serena (symbole) zamiast czytania całych plików.
- Po istotnym etapie zaktualizuj `PROJECT_STATUS.md` i `README_FIRST.txt`.
