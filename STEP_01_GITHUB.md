# Etap 9 — X4 MultiHub v0.9-rc1

To jest pierwszy release candidate.

1. Rozpakuj ZIP.
2. Skopiuj cala zawartosc do repo `X4-MultiHub-X4`.
3. Potwierdz nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v0.9-rc1 release candidate`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions.
8. Workflow nazywa sie teraz:
   `BUILD X4 MULTIHUB RC BIN`
9. Po Success pobierz artifact:
   `X4_MultiHub_X4_v0_9_rc1`

Oczekiwany BIN:
`X4_MultiHub_X4_v0.9-rc1.bin`

RC dodaje dodatkowe bramki:
- audit zrodel,
- brak automatycznego erase_flash,
- pelna analiza struktury ESP BIN,
- wewnetrzny checksum ESP,
- appended SHA-256,
- brak dodatkowego ogona danych,
- wszystkie markery funkcji w finalnym BIN,
- fit do potwierdzonej partycji aplikacji,
- raport wolnego miejsca.

To nadal jest APPLICATION BIN, nie merged/full-flash BIN.

Status fizycznego X4 pozostaje:
HARDWARE UNVERIFIED
