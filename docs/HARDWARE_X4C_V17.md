# X4 MultiHub v1.7 - X4 Classic / ESP32-S3 hardware target

## Verified physical device

- MCU: ESP32-S3 QFN56, revision v0.2
- PSRAM: 8 MB
- Flash: 16 MB
- USB: USB Serial/JTAG
- Full pre-flash backup: two independent 16 MiB reads were byte-identical by SHA-256
- Backup SHA-256:
  DEF3CF72FA4D8FCFB4DBE6E1EB34002B6B1B2A754917A11709A62DC1D18B22DF

## Physical partition table recovered from device

| Partition | Offset | Size |
| --- | ---: | ---: |
| nvs | 0x009000 | 0x005000 |
| otadata | 0x00E000 | 0x002000 |
| app0 | 0x010000 | 0x7E0000 |
| app1 | 0x7F0000 | 0x7E0000 |
| spiffs | 0xFD0000 | 0x014000 |
| coredump | 0xFE4000 | 0x01C000 |

## Build strategy

The application is rebuilt on official CrossPoint 1.6.5 using the dedicated:

- PlatformIO environment: x4c-gh_release
- board: esp32-s3-devkitc1-n16r8
- MCU: esp32s3
- device macro: FREEINK_DEVICE_X4CLASSIC=1
- PSRAM flag: BOARD_HAS_PSRAM
- SD mode: USE_BLOCK_DEVICE_INTERFACE=1

The generated deliverable remains an APPLICATION BIN, not a merged/full-flash image.

The CrossPoint web flasher reads the partition table from the physical device,
writes the application to the inactive OTA slot and updates otadata. Therefore
the project does not replace the recovered factory partition table during a
normal Custom .bin installation.

## Release gate

A build is rejected unless:

- ESP image chip_id_raw == 9 (ESP32-S3),
- board == esp32-s3-devkitc1-n16r8,
- MCU == esp32s3,
- FREEINK_DEVICE_X4CLASSIC=1 is present,
- internal ESP checksum is correct,
- appended ESP SHA-256 is correct when present,
- image fits the CrossPoint build app slot,
- image also fits the physical 0x7E0000 OTA slot,
- all v1.7 MultiHub feature markers are present.

The first successful build is named `v1.7-hwtest`. It must not be renamed
"stable" until it boots and passes the physical smoke test.
