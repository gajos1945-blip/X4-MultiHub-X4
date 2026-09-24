# X4 MultiHub v1.1-dev — News Terminal

1. Rozpakuj ZIP.
2. Skopiuj cala zawartosc do obecnego repo `X4-MultiHub-X4`.
3. Potwierdz nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v1.1-dev News Terminal`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions.
8. Workflow:
   `BUILD X4 MULTIHUB v1.1 DEV BIN`
9. Po Success pobierz artifact:
   `X4_MultiHub_X4_v1_1_dev`

Oczekiwany BIN:
`X4_MultiHub_X4_v1.1-dev.bin`

Nowy modul:
- Wiadomosci / News Terminal,
- dowolne publiczne RSS/Atom dodawane z klawiatury,
- wlasna lista zrodel na microSD,
- naglowki / zrodlo / data publikacji,
- ulubione artykuly,
- otwarcie tresci/summary z feedu w standardowym readerze CrossPoint,
- gateway endpoint `/v1/news`,
- ochrona SSRF: prywatne/loopback/link-local/reserved IP sa blokowane,
- redirecty sa walidowane ponownie,
- limit 1 MiB na feed i max 20 artykulow.

Celowo NOT IMPLEMENTED w v1.1-dev:
- pelne pobieranie i "readability" calej strony WWW artykulu,
- offline cache dla zwyklych, nieulubionych kanalow,
- Power Manager,
- NTP/automatyczne Today.

Nie flashujemy tego development milestone bez osobnej decyzji testowej.
