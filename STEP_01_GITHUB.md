# X4 MultiHub v1.2-dev — Power Manager

1. Rozpakuj ZIP.
2. Skopiuj cala zawartosc do tego samego repo `X4-MultiHub-X4`.
3. Potwierdz nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v1.2-dev Power Manager`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions.
8. Workflow:
   `BUILD X4 MULTIHUB v1.2 DEV BIN`
9. Po Success pobierz artifact:
   `X4_MultiHub_X4_v1_2_dev`

Oczekiwany BIN:
`X4_MultiHub_X4_v1.2-dev.bin`

Power Manager:
- Ustawienia MultiHub -> Power Manager
- Online przy otwarciu:
  RECZNIE (domyslnie) / AUTO
- Wi-Fi OFF po odswiezeniu:
  WYLACZONE (domyslnie) / WLACZONE
- Wylacz Wi-Fi teraz:
  jawna akcja uzytkownika

Wylaczenie radia korzysta z Arduino-ESP32:
`WiFi.disconnect(true, false)`
czyli wifioff=true i eraseap=false.

Nie implementujemy jeszcze:
- zmian deep sleep / wake sources,
- progow baterii,
bo wymagaja fizycznego X4.

To nadal DEVELOPMENT + APPLICATION BIN.
Workflow nie wykonuje erase_flash i nie flashuje urzadzenia.
