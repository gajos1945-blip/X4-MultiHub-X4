X4 MULTIHUB — PROJECT 2
v0.5-dev
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

STATUS v0.5-dev
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

NOT IMPLEMENTED w v0.5-dev:
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


v0.5-dev: Field Manual jest teraz IMPLEMENTED; szczegoly w STEP_01_GITHUB.md.


v0.5-dev: Daily Planner jest teraz IMPLEMENTED; automatyczna data RTC/NTP pozostaje jawnie odlozona.


v0.5-dev: Markets jest teraz IMPLEMENTED; Weather UI i Dashboard pozostaja kolejnym etapem.
