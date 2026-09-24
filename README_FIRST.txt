X4 MULTIHUB — PROJECT 2
v1.0.0-rc1
=======================

CEL
---
Jeden firmware dla XTEINK X4, zbudowany na stabilnej bazie CrossPoint,
z czterema glownymi modulami:

1. Reader
2. Field Manual
3. Daily Planner
4. Markets & Weather

Dodatkowo:
- Dashboard
- Ustawienia
- dynamiczne ulubione instrumenty
- GPW / NewConnect / Forex / Crypto
- pogoda online
- dane przechowywane i cache'owane na microSD

STATUS v1.0.0-rc1
---------------
IMPLEMENTED:
- Reader w MultiHub otwiera stockowa biblioteke / przegladarke plikow CrossPoint
- bezpieczny builder GitHub Actions
- przypieta baza CrossPoint 1.6.0
- X4 MultiHub jako osobna pozycja na Home
- ekran glowny X4 MultiHub
- oznaczenia NOT IMPLEMENTED dla modulow, ktore dopiero powstana
- X4 Data Gateway
- EODHD provider: katalog WAR, katalog CC, quote
- Open-Meteo provider: geocoding + current weather
- testy gatewaya i buildera
- fail-closed weryfikacja flash/offset/partitions przed publikacja BIN

NOT IMPLEMENTED w v1.0.0-rc1:
- Field Manual
- Daily Planner
- portfolio/favorites na X4
- finalny Markets UI
- finalny Dashboard
- finalne Ustawienia MultiHub

UWAGA
-----
Nie wpisujemy na sztywno flash size, offsetow ani partycji.
CI odczytuje je ponownie z przypietego projektu CrossPoint i blokuje build,
jesli krytyczne wartosci sa UNKNOWN.

Ten workflow nie wykonuje erase_flash i nie flashuje urzadzenia.


v1.0.0-rc1: Field Manual jest teraz IMPLEMENTED; szczegoly w STEP_01_GITHUB.md.


v1.0.0-rc1: Daily Planner jest teraz IMPLEMENTED; automatyczna data RTC/NTP pozostaje jawnie odlozona.


v1.0.0-rc1: Markets jest teraz IMPLEMENTED; Weather UI i Dashboard pozostaja kolejnym etapem.


v1.0.0-rc1: Weather i Dashboard sa teraz IMPLEMENTED. Cache/resilience pozostaja etapem v0.7.


v1.0.0-rc1: cache i resilience sa IMPLEMENTED; centralne Settings pozostaje kolejnym etapem.


v1.0.0-rc1: centralne Ustawienia MultiHub i glowne UX sa IMPLEMENTED. Nastepny etap: v0.9-rc.


v1.0.0-rc1: RELEASE CANDIDATE. Static/CI release gates sa zaostrzone. Physical X4 validation: NOT RUN.


v1.0.0-rc1: SOFTWARE FEATURE COMPLETE. Physical X4 validation: NOT RUN.


v1.1-dev: rozpoczecie dalszego rozwoju na bazie zamrozonego v1.0.0-rc1. Nowy modul: News Terminal RSS/Atom.


v1.2-dev: Power Manager. Domyslnie brak automatycznego requestu News przy otwarciu. Opcjonalne Wi-Fi OFF po skonczonej operacji online jest domyslnie WYLACZONE.


v1.3-dev: Time / NTP / Today. Manual SNTP, jawna strefa czasowa, opcjonalne Auto Today i wiek cache. Brak deklaracji o fizycznym RTC X4.


v1.4-dev: Reader Dashboard. Czyta stan i progress CrossPoint bez modyfikowania progress.bin; dodaje Kontynuuj czytanie i karte Czytnik w Dashboard.


v1.6-dev: Gateway Access Control. Opcjonalny wspolny token dla /v1/*, X-X4-Token na X4, DPAPI w Windows Gateway. HTTP w LAN pozostaje nieszyfrowane.


v1.7-dev: Diagnostyka. Test Wi-Fi/Gateway/auth/microSD/czasu, lokalny /v1/ping i eksport raportu z tokenem REDACTED.
