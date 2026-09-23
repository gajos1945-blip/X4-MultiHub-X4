# Etap 2 — X4 MultiHub v0.2-dev Reader integration

1. Rozpakuj ZIP.
2. Skopiuj całą zawartość folderu do lokalnego repo `X4-MultiHub-X4`.
3. Potwierdź nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v0.2-dev Reader integration`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions -> `BUILD X4 MULTIHUB DEV BIN`.
8. Po Success pobierz artifact:
   `X4_MultiHub_X4_v0_2_dev`

Oczekiwany BIN:
`X4_MultiHub_X4_v0.2-dev.bin`

Zmiana funkcjonalna:
`Reader` w X4 MultiHub nie jest już placeholderem. Wywołuje potwierdzoną
nawigację CrossPoint `activityManager.goToFileBrowser()`. Wybór pliku w
standardowej przeglądarce pozostaje obsługiwany przez bazę CrossPoint.

Pozostałe moduły są nadal jawnie oznaczone `NOT IMPLEMENTED v0.2`.
Nie flashujemy tej wersji bez osobnej decyzji o teście milestone.
