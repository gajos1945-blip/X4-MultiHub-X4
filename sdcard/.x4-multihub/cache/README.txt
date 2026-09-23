X4 MultiHub cache v0.7

Runtime:
  /.x4-multihub/cache/market_quotes.json
  /.x4-multihub/cache/weather.json

Zasady:
- po poprawnym odswiezeniu dane zapisywane sa na microSD,
- przy starcie ostatni cache moze zostac pokazany od razu,
- przy bledzie Wi-Fi/gateway/provider system pozostawia ostatni cache,
- UI jawnie pokazuje LIVE albo CACHED,
- brak danych i brak cache = DATA UNAVAILABLE.

Wiek cache:
Firmware zachowuje timestamp dostawcy dla notowan i observed_at dla pogody.
Nie liczy jeszcze uczciwego "X minut temu", poniewaz zachowanie RTC/NTP
na fizycznym X4 nie zostalo zweryfikowane.
