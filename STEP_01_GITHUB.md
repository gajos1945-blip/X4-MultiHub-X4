# Etap 7 — X4 MultiHub v0.7-dev Cache + Resilience

1. Rozpakuj ZIP.
2. Skopiuj cala zawartosc do lokalnego repo `X4-MultiHub-X4`.
3. Potwierdz nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v0.7-dev Cache Resilience`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions -> `BUILD X4 MULTIHUB DEV BIN`.
8. Po Success pobierz artifact:
   `X4_MultiHub_X4_v0_7_dev`

Oczekiwany BIN:
`X4_MultiHub_X4_v0.7-dev.bin`

Nowe funkcje:
- cache notowan na microSD,
- cache pogody i prognozy na microSD,
- cache-first przy otwarciu Markets / Pogoda / Dashboard,
- fallback do cache po bledzie Wi-Fi/gateway/provider,
- jasne oznaczenia LIVE / CACHED / DATA UNAVAILABLE,
- timestamp dostawcy przy notowaniach,
- observed_at przy pogodzie,
- atomowy zapis cache przez plik .tmp + rename,
- limit rozmiaru cache,
- gateway retry/backoff dla 429, 500, 502, 503, 504 oraz bledow polaczenia.

Celowo nadal nie liczymy "X minut temu" na X4.
Do tego potrzebujemy zweryfikowanego RTC/NTP na fizycznym urzadzeniu.

Nie flashujemy development milestone bez osobnej decyzji testowej.
