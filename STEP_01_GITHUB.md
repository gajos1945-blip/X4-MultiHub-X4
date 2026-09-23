# Finalny etap — X4 MultiHub v1.0.0-rc1

1. Rozpakuj ZIP.
2. Skopiuj cala zawartosc do repo `X4-MultiHub-X4`.
3. Potwierdz nadpisanie.
4. GitHub Desktop -> Summary:
   `X4 MultiHub v1.0.0-rc1 final software candidate`
5. Commit to main.
6. Push origin.
7. GitHub -> Actions.
8. Workflow:
   `BUILD X4 MULTIHUB FINAL RC BIN`
9. Po Success pobierz artifact:
   `X4_MultiHub_X4_v1_0_0_rc1`

W artifact:
- X4_MultiHub_X4_v1.0.0-rc1.bin
- X4_MultiHub_X4_v1.0.0-rc1.bin.sha256.txt
- Custom.bin
- Custom.bin.sha256.txt
- build_manifest.json
- source_report.json
- FINAL_STATUS.txt

Custom.bin i nazwany release BIN musza miec identyczny SHA-256.

To APPLICATION BIN, nie merged/full-flash.
Status fizycznego X4 pozostaje HARDWARE UNVERIFIED do czasu testu urzadzenia.
