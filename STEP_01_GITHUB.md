# X4 MultiHub v1.7-dev — Diagnostics

1. Rozpakuj ZIP.
2. Skopiuj cala zawartosc do tego samego repo `X4-MultiHub-X4`.
3. Potwierdz nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v1.7-dev Diagnostics`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions.
8. Workflow:
   `BUILD X4 MULTIHUB v1.7 DEV BIN`

Po Success beda dwa artifacty:

Firmware:
  `X4_MultiHub_X4_v1_7_dev`
  plik: `X4_MultiHub_X4_v1.7-dev.bin`

Windows Gateway:
  `X4_Data_Gateway_Windows_v1_7_dev`
  plik: `X4DataGateway.exe`

Nowosc v1.7:
- Ustawienia -> Diagnostyka
- Wi-Fi/RSSI
- Gateway /health
- Gateway auth /v1/ping
- test zapisu/odczytu/usuniecia pliku tymczasowego na microSD
- status czasu lokalnego
- eksport:
  /.x4-multihub/diagnostics/last_report.txt
- raport NIGDY nie zapisuje wartosci Gateway access token
- Windows Gateway ma przycisk "Test X4 auth"

`/v1/ping` jest lokalne i nie odpytuje EODHD, Open-Meteo ani RSS.

Firmware nadal jest DEVELOPMENT + APPLICATION BIN.
Workflow nie wykonuje erase_flash i nie flashuje urzadzenia.
