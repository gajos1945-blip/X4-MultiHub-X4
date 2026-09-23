# Etap 5 — X4 MultiHub v0.5-dev Markets

1. Rozpakuj ZIP.
2. Skopiuj cala zawartosc do lokalnego repo `X4-MultiHub-X4`.
3. Potwierdz nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v0.5-dev Markets`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions -> `BUILD X4 MULTIHUB DEV BIN`.
8. Po Success pobierz artifact:
   `X4_MultiHub_X4_v0_5_dev`

Oczekiwany BIN:
`X4_MultiHub_X4_v0.5-dev.bin`

Nowe funkcje:
- realny ekran Markets,
- ustawianie adresu X4 Data Gateway,
- dynamiczne wyszukiwanie GPW/NewConnect z katalogu WAR,
- dynamiczne wyszukiwanie crypto z katalogu CC,
- dynamiczne wyszukiwanie walut z katalogu FOREX,
- wyszukiwanie po nazwie/tickerze/ISIN,
- ulubione na microSD,
- batch refresh notowan,
- cena + zmiana procentowa,
- DATA UNAVAILABLE zamiast wymyslonej wartosci,
- dlugie przytrzymanie usuwa ulubiony instrument.

Gateway:
- klucz EODHD pozostaje na komputerze,
- dodany endpoint /v1/quotes,
- kilka symboli pobieranych jednym requestem EODHD `s=...`.

Wazne:
EODHD Live/Delayed API jest zalezne od planu i rynku. Firmware nie nazywa
tych danych real-time. Pokazuje tylko wartosc zwrocona przez gateway.
Weather UI powstanie w v0.6.

Nie flashujemy development milestone bez osobnej decyzji testowej.
