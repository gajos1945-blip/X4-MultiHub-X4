# Etap 6 — X4 MultiHub v0.6-dev Weather + Dashboard

1. Rozpakuj ZIP.
2. Skopiuj cala zawartosc do lokalnego repo `X4-MultiHub-X4`.
3. Potwierdz nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v0.6-dev Weather Dashboard`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions -> `BUILD X4 MULTIHUB DEV BIN`.
8. Po Success pobierz artifact:
   `X4_MultiHub_X4_v0_6_dev`

Oczekiwany BIN:
`X4_MultiHub_X4_v0.6-dev.bin`

Nowe funkcje:
- osobny ekran Pogoda,
- miasto ustawiane z klawiatury,
- Open-Meteo przez X4 Data Gateway,
- temperatura / odczuwalna / wilgotnosc / opad / wiatr,
- kod warunkow WMO,
- prognoza do 4 dni,
- prawdziwy Dashboard,
- Dashboard laczy Pogode + Rynki + Planner,
- widocznosc kart konfigurowalna,
- kolejnosc kart zapisywana na microSD,
- dlugie przytrzymanie karty w Uklad Dashboard przesuwa ja wyzej,
- maksymalnie 6 pierwszych ulubionych instrumentow na Dashboard.

Celowo nadal NOT IMPLEMENTED:
- cache offline i stale-data indicator,
- retry/backoff,
- centralny ekran MultiHub Settings,
- automatyczna data Today z RTC/NTP,
- karta postepu aktualnie czytanej ksiazki.

Nie flashujemy development milestone bez osobnej decyzji testowej.
