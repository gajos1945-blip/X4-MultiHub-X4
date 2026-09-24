# X4 MultiHub v1.6-dev — Gateway Access Control

1. Rozpakuj ZIP.
2. Skopiuj cala zawartosc do tego samego repo `X4-MultiHub-X4`.
3. Potwierdz nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v1.6-dev Gateway Access Control`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions.
8. Workflow:
   `BUILD X4 MULTIHUB v1.6 DEV BIN`

Po Success beda dwa artifacty:

Firmware:
  `X4_MultiHub_X4_v1_6_dev`
  plik: `X4_MultiHub_X4_v1.6-dev.bin`

Windows Gateway:
  `X4_Data_Gateway_Windows_v1_6_dev`
  plik: `X4DataGateway.exe`

Nowosc v1.6:
- Windows Gateway moze wygenerowac Gateway access token,
- jesli token jest ustawiony, /v1/* wymaga X-X4-Token,
- ten sam token wpisujesz na X4:
    Ustawienia -> Gateway access token
- /health pozostaje publiczne i nie ujawnia tokenu,
- X4 przechowuje token na microSD, nie w skompilowanym firmware,
- Windows przechowuje oba sekrety przez DPAPI.

Wazne:
- token to kontrola dostepu, nie szyfrowanie,
- transport X4 <-> Gateway nadal jest HTTP w LAN,
- uzywaj tylko w zaufanej sieci prywatnej.

Firmware nadal jest DEVELOPMENT + APPLICATION BIN.
Workflow nie wykonuje erase_flash i nie flashuje urzadzenia.
