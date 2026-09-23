# X4 MultiHub v0.9-rc1 — release candidate checklist

## Static / CI gates

The build must block unless all are true:

- pinned CrossPoint commit matches,
- flash size is confirmed from `platformio.ini`,
- application offset is confirmed from `platformio.ini`,
- partition table is parsed from `partitions.csv`,
- no partition overlap,
- `firmware.bin` is a valid ESP image,
- internal ESP image checksum matches,
- appended ESP SHA-256 matches when present,
- there are no unexplained trailing bytes,
- application BIN fits the confirmed `app0` partition,
- required feature markers exist in the final BIN,
- RC source audit passes,
- workflow contains no automatic `erase_flash`,
- artifact is explicitly marked `merged_full_flash: false`.

## Hardware status

This RC is **HARDWARE UNVERIFIED** until a physical XTEINK X4 is available.

First-device smoke test:

1. Keep the stock recovery/update firmware available.
2. Use the CrossPoint Custom .bin path only for the application BIN.
3. Do not erase flash automatically.
4. Boot X4.
5. Open X4 MultiHub.
6. Check Czytnik.
7. Check Instrukcje / Field Manual.
8. Check Planer.
9. Check Rynki.
10. Check Pogoda.
11. Check Dashboard.
12. Check Ustawienia.
13. Verify microSD writes.
14. Verify Wi-Fi + gateway.
15. Verify cached fallback after disconnecting network.

Only after these checks may hardware status be changed from UNKNOWN/UNVERIFIED.
