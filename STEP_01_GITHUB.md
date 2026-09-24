# X4 MultiHub v1.5-dev — Windows Gateway

1. Rozpakuj ZIP.
2. Skopiuj cala zawartosc do tego samego repo `X4-MultiHub-X4`.
3. Potwierdz nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v1.5-dev Windows Gateway`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions.
8. Workflow:
   `BUILD X4 MULTIHUB v1.5 DEV BIN`

Po Success beda dwa artifacty:

1. Firmware:
   `X4_MultiHub_X4_v1_5_dev`

   BIN:
   `X4_MultiHub_X4_v1.5-dev.bin`

2. Windows Gateway:
   `X4_Data_Gateway_Windows_v1_5_dev`

   EXE:
   `X4DataGateway.exe`

Gateway EXE:
- Start / Stop,
- pokazuje adres LAN dla X4,
- /health,
- EODHD token chroniony Windows DPAPI,
- token nie trafia do firmware ani GitHub.

Windows Firewall moze poprosic o zgode przy pierwszym uruchomieniu.
Zezwalaj tylko w zaufanej sieci prywatnej/LAN.
Nie wystawiaj gateway bezposrednio do Internetu.

Firmware nadal jest DEVELOPMENT + APPLICATION BIN.
Workflow nie wykonuje erase_flash i nie flashuje urzadzenia.
