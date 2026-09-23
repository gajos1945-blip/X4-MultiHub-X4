# Etap 4 — X4 MultiHub v0.4-dev Daily Planner

1. Rozpakuj ZIP.
2. Skopiuj cala zawartosc do lokalnego repo `X4-MultiHub-X4`.
3. Potwierdz nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v0.4-dev Daily Planner`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions -> `BUILD X4 MULTIHUB DEV BIN`.
8. Po Success pobierz artifact:
   `X4_MultiHub_X4_v0_4_dev`

Oczekiwany BIN:
`X4_MultiHub_X4_v0.4-dev.bin`

Nowe funkcje:
- prawdziwy Daily Planner,
- trwałe zadania w JSONL na microSD,
- aktywny dzien YYYY-MM-DD,
- poprzedni / nastepny dzien,
- dodawanie z klawiatury,
- status otwarte / wykonane,
- widoki Wszystkie / Otwarte / Wykonane,
- priorytet wysoki / normalny / niski,
- notatka,
- przeniesienie zadania na inna date,
- usuwanie.

Wazne:
Automatyczne pobieranie "dzisiaj" z RTC/NTP jest nadal NOT IMPLEMENTED.
v0.4 wymaga recznego ustawienia aktywnej daty. To celowe: nie zgadujemy
zachowania zegara bez testu na fizycznym X4.

Nie flashujemy development milestone bez osobnej decyzji testowej.
