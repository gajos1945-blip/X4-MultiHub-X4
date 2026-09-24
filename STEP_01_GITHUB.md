# X4 MultiHub v1.3-dev — Time / NTP / Today

1. Rozpakuj ZIP.
2. Skopiuj cala zawartosc do tego samego repo `X4-MultiHub-X4`.
3. Potwierdz nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v1.3-dev Time NTP Today`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions.
8. Workflow:
   `BUILD X4 MULTIHUB v1.3 DEV BIN`
9. Po Success pobierz artifact:
   `X4_MultiHub_X4_v1_3_dev`

Oczekiwany BIN:
`X4_MultiHub_X4_v1.3-dev.bin`

Nowe funkcje:
- Ustawienia -> Czas / NTP / Today
- reczna synchronizacja SNTP
- strefy: UTC / Polska / UK / US Eastern
- Auto Today dla Planera (domyslnie WYLACZONE)
- wiek cache dla Rynkow, Pogody i Dashboard
- stary cache bez timestampu nadal dziala i pokazuje po prostu CACHED

Bezpieczne zalozenia:
- SNTP dziala tylko przy juz polaczonym Wi-Fi
- MultiHub nie kasuje zapisanych danych Wi-Fi
- nie twierdzimy, ze X4 utrzymuje dokladny czas przez kazdy sleep/power cycle
  bez testu fizycznego urzadzenia

To nadal DEVELOPMENT + APPLICATION BIN.
Workflow nie wykonuje erase_flash i nie flashuje urzadzenia.
