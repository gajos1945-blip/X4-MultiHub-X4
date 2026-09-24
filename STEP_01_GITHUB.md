# X4 MultiHub v1.4-dev — Reader Dashboard

1. Rozpakuj ZIP.
2. Skopiuj cala zawartosc do tego samego repo `X4-MultiHub-X4`.
3. Potwierdz nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v1.4-dev Reader Dashboard`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions.
8. Workflow:
   `BUILD X4 MULTIHUB v1.4 DEV BIN`
9. Po Success pobierz artifact:
   `X4_MultiHub_X4_v1_4_dev`

Oczekiwany BIN:
`X4_MultiHub_X4_v1.4-dev.bin`

Reader Dashboard:
- Dashboard dostaje karte Czytnik,
- pokazuje ostatnio/biezaco otwarta ksiazke z CrossPoint APP_STATE,
- tytul i autor pochodza z CrossPoint RecentBooksStore,
- dla EPUB czytany jest CrossPoint progress.bin (read-only),
- pokazuje strone rozdzialu i procent rozdzialu,
- jesli cache EPUB pozwala: pokazuje tez procent calej ksiazki,
- klik karty -> Reader Dashboard,
- Kontynuuj czytanie -> stockowy reader CrossPoint,
- brak ksiazki -> Biblioteka i pliki.

MultiHub NIE zapisuje ani nie zmienia CrossPoint progress.bin.

To nadal DEVELOPMENT + APPLICATION BIN.
Workflow nie wykonuje erase_flash i nie flashuje urzadzenia.
