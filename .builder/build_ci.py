from __future__ import annotations
from pathlib import Path
import hashlib
import json
import shutil
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT / ".builder" / "tools"
sys.path.insert(0, str(TOOLS))

from patch_crosspoint import apply as apply_patch, PatchError
from verify_project import inspect as inspect_project, VerificationError
from bin_inspector import inspect_bin
from release_guard import verify_required_markers
from rc_audit import audit_repository

UPSTREAM = "https://github.com/crosspoint-reader/crosspoint-reader.git"

# CrossPoint 1.6.5 is the first stable base in this project line with official
# XTEINK X4 Classic / ESP32-S3 support.
COMMIT = "93e98bb78702e29868a16a13b80c40e6b36ccdff"
BASE_RELEASE = "CrossPoint 1.6.5"

# IMPORTANT: the user's physical reader was identified as ESP32-S3 with 8 MB
# PSRAM and 16 MB flash. Never build this package with the C3 gh_release env.
ENV = "x4c-gh_release"
EXPECTED_BOARD = "esp32-s3-devkitc1-n16r8"
EXPECTED_MCU = "esp32s3"
EXPECTED_CHIP_ID_RAW = 9
EXPECTED_DEVICE_MACRO = "FREEINK_DEVICE_X4CLASSIC=1"

RELEASE_NAME = "X4_MultiHub_X4C_v1.7-hwtest.bin"

# Verified physical-device facts from the pre-flash backup session.
PHYSICAL_PROFILE = {
    "device_family": "XTEINK X4 Classic / X4C",
    "chip": "ESP32-S3",
    "chip_revision": "v0.2",
    "psram_bytes": 8 * 1024 * 1024,
    "flash_bytes": 16 * 1024 * 1024,
    "backup_sha256": "DEF3CF72FA4D8FCFB4DBE6E1EB34002B6B1B2A754917A11709A62DC1D18B22DF",
    "partition_table": [
        {"name": "nvs", "offset": 0x9000, "size": 0x5000},
        {"name": "otadata", "offset": 0xE000, "size": 0x2000},
        {"name": "app0", "offset": 0x10000, "size": 0x7E0000},
        {"name": "app1", "offset": 0x7F0000, "size": 0x7E0000},
        {"name": "spiffs", "offset": 0xFD0000, "size": 0x14000},
        {"name": "coredump", "offset": 0xFE4000, "size": 0x1C000},
    ],
}

class BuildError(RuntimeError):
    pass


def run(args: list[str], cwd: Path | None = None) -> str:
    p = subprocess.run(
        args,
        cwd=str(cwd) if cwd else None,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        shell=False,
    )
    print(p.stdout, end="")
    if p.returncode != 0:
        raise BuildError(f"Command failed ({p.returncode}): {args!r}")
    return p.stdout


def require(name: str) -> str:
    p = shutil.which(name)
    if not p:
        raise BuildError(f"Required tool not found: {name}")
    return p


def parse_flash_bytes(value: str) -> int:
    s = value.strip().upper()
    for suffix, mult in (
        ("MB", 1024 * 1024),
        ("KB", 1024),
        ("M", 1024 * 1024),
        ("K", 1024),
    ):
        if s.endswith(suffix):
            return int(s[:-len(suffix)]) * mult
    return int(s, 0)


def main() -> int:
    git = require("git")
    pio = require("pio")
    work = ROOT / "_work" / "crosspoint-reader"
    dist = ROOT / "dist"

    if work.exists():
        shutil.rmtree(work)
    if dist.exists():
        shutil.rmtree(dist)
    work.parent.mkdir(parents=True, exist_ok=True)
    dist.mkdir(parents=True, exist_ok=True)

    print("=== 0/10 Project source audit ===")
    audit_errors = audit_repository(ROOT)
    if audit_errors:
        raise BuildError("; ".join(audit_errors))

    print("=== 1/10 Clone pinned CrossPoint 1.6.5 ===")
    run([git, "clone", "--recursive", UPSTREAM, str(work)])
    run([git, "checkout", "--detach", COMMIT], cwd=work)
    run([git, "submodule", "sync", "--recursive"], cwd=work)
    run([git, "submodule", "update", "--init", "--recursive"], cwd=work)
    actual = run([git, "rev-parse", "HEAD"], cwd=work).strip()
    if actual.lower() != COMMIT.lower():
        raise BuildError(f"Wrong CrossPoint commit: {actual}")

    print("=== 2/10 Verify X4C build environment ===")
    report = inspect_project(work, ENV)
    if report["flash_size"] == "UNKNOWN":
        raise BuildError("Flash size UNKNOWN")
    if report["application_offset"] == "UNKNOWN":
        raise BuildError("Application offset UNKNOWN")
    if report["partition_file"] == "UNKNOWN":
        raise BuildError("Partition table UNKNOWN")
    if report.get("board") != EXPECTED_BOARD:
        raise BuildError(
            f"Wrong board for physical device: {report.get('board')} != {EXPECTED_BOARD}"
        )
    if report.get("mcu") != EXPECTED_MCU:
        raise BuildError(
            f"Wrong MCU for physical device: {report.get('mcu')} != {EXPECTED_MCU}"
        )
    if EXPECTED_DEVICE_MACRO not in report.get("build_flags", ""):
        raise BuildError(
            f"Missing X4 Classic device macro: {EXPECTED_DEVICE_MACRO}"
        )

    print("=== 3/10 Apply X4 MultiHub v1.7 layer ===")
    apply_patch(work, ROOT / ".builder" / "overlay")

    # Re-inspect after patching so the report describes the exact source that
    # will be compiled.
    report = inspect_project(work, ENV)
    (dist / "source_report.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )

    print("=== 4/10 Build ESP32-S3 X4 Classic release environment ===")
    started = time.time()
    run([pio, "run", "-e", ENV, "-j1"], cwd=work)
    elapsed = round(time.time() - started, 3)

    print("=== 5/10 Validate ESP application image ===")
    fw = work / ".pio" / "build" / ENV / "firmware.bin"
    if not fw.is_file():
        raise BuildError("Build succeeded but firmware.bin is missing")

    info = inspect_bin(fw)
    if not info["esp_image"]:
        raise BuildError("Output is not an ESP application image")
    if not info["structure_valid"]:
        raise BuildError(
            "ESP image integrity validation failed: "
            + "; ".join(info.get("errors", []))
        )
    if info["image_checksum_valid"] is not True:
        raise BuildError("ESP image checksum mismatch")
    if info["hash_appended"] and info["appended_sha256_valid"] is not True:
        raise BuildError("ESP appended SHA-256 mismatch")
    if info["trailing_bytes"] != 0:
        raise BuildError("Unexpected trailing bytes after ESP image")
    if info.get("chip_id_raw") != EXPECTED_CHIP_ID_RAW:
        raise BuildError(
            f"WRONG CHIP IMAGE: chip_id_raw={info.get('chip_id_raw')} "
            f"(expected {EXPECTED_CHIP_ID_RAW} for ESP32-S3)"
        )

    print("=== 6/10 Validate flash and application slot ranges ===")
    flash_bytes = parse_flash_bytes(report["flash_size"])
    app_offset = int(report["application_offset"], 0)
    if flash_bytes != PHYSICAL_PROFILE["flash_bytes"]:
        raise BuildError(
            f"Build flash size {flash_bytes} does not match verified device "
            f"flash {PHYSICAL_PROFILE['flash_bytes']}"
        )
    if app_offset + info["size"] > flash_bytes:
        raise BuildError("Application image exceeds confirmed flash range")

    app_partition = None
    for part in report.get("partitions", []):
        if part.get("type") == "app" and part.get("offset") == app_offset:
            app_partition = part
            break

    if not app_partition or app_partition.get("size") is None:
        raise BuildError("Application partition size UNKNOWN; release blocked")

    # The pinned CrossPoint build layout uses 0x640000 app slots. The user's
    # stock device has larger 0x7E0000 OTA slots. Requiring the image to fit
    # BOTH layouts keeps the application BIN valid for the CrossPoint build
    # and for the real device table used by the web flasher.
    build_slot_size = int(app_partition["size"])
    physical_slot_size = 0x7E0000
    if info["size"] > build_slot_size:
        raise BuildError(
            f"Application image ({info['size']}) exceeds CrossPoint build "
            f"app partition ({build_slot_size})"
        )
    if info["size"] > physical_slot_size:
        raise BuildError(
            f"Application image ({info['size']}) exceeds physical X4C "
            f"OTA slot ({physical_slot_size})"
        )

    print("=== 7/10 Verify MultiHub feature markers ===")
    marker_errors = verify_required_markers(fw)
    if marker_errors:
        raise BuildError("; ".join(marker_errors))

    partition_spare = build_slot_size - int(info["size"])
    physical_spare = physical_slot_size - int(info["size"])
    if partition_spare < 0 or physical_spare < 0:
        raise BuildError("Negative application partition spare space")

    print("=== 8/10 Create hardware-test artifact ===")
    final_bin = dist / RELEASE_NAME
    shutil.copy2(fw, final_bin)

    sha = hashlib.sha256(final_bin.read_bytes()).hexdigest()
    (dist / (RELEASE_NAME + ".sha256.txt")).write_text(
        f"{sha}  {RELEASE_NAME}\n", encoding="ascii"
    )

    manifest = {
        "project": "X4 MultiHub",
        "version": "1.7-hwtest",
        "target": "XTEINK X4 Classic / ESP32-S3",
        "base_release": BASE_RELEASE,
        "upstream_commit": COMMIT,
        "platformio_environment": ENV,
        "build_status": "SUCCESS",
        "build_seconds": elapsed,
        "artifact": {**info, "filename": RELEASE_NAME, "sha256": sha},
        "source_facts": report,
        "physical_device_profile": PHYSICAL_PROFILE,
        "application_partition_build": app_partition,
        "application_partition_physical": {
            "app0_offset": 0x10000,
            "app1_offset": 0x7F0000,
            "slot_size": physical_slot_size,
        },
        "release_status": "X4C_V1_7_HARDWARE_TEST_CANDIDATE",
        "software_feature_complete": True,
        "image_integrity_verified": True,
        "chip_target_verified": True,
        "application_partition_spare_bytes_build": partition_spare,
        "application_partition_spare_bytes_physical": physical_spare,
        "rc_source_audit_verified": True,
        "required_feature_markers_verified": True,
        "physical_device_verified": False,
        "flash_performed": False,
        "merged_full_flash": False,
        "automatic_erase": False,
    }
    (dist / "build_manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )

    (dist / "DEV_STATUS.txt").write_text(
        "X4 MultiHub v1.7-hwtest\n"
        "TARGET: XTEINK X4 Classic / ESP32-S3\n"
        "BASE: CrossPoint 1.6.5\n"
        "ENV: x4c-gh_release\n"
        "IMAGE TYPE: APPLICATION BIN\n"
        "ESP IMAGE CHIP ID: 9 (ESP32-S3)\n"
        "MERGED/FULL FLASH: NO\n"
        "AUTOMATIC ERASE: NO\n"
        "PRE-FLASH FULL BACKUP: VERIFIED\n"
        "PHYSICAL BOOT/SMOKE TEST: NOT RUN YET\n"
        f"SHA-256: {sha}\n",
        encoding="utf-8",
    )

    print("=== 9/10 Safety gates PASS ===")
    print("Target board:", report["board"])
    print("Target MCU:", report["mcu"])
    print("ESP image chip id:", info["chip_id_raw"])
    print("Image checksum: PASS")
    print("Appended SHA-256:", "PASS" if info["appended_sha256_valid"] else "N/A")
    print("Required feature markers: PASS")
    print("Build app-slot spare bytes:", partition_spare)
    print("Physical app-slot spare bytes:", physical_spare)

    print("=== 10/10 DONE ===")
    print("BIN:", final_bin)
    print("SHA-256:", sha)
    print("Status: X4C v1.7 HARDWARE TEST CANDIDATE")
    print("This workflow does NOT flash or erase any device.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (BuildError, PatchError, VerificationError, ValueError) as exc:
        print("BUILD BLOCKED:", exc, file=sys.stderr)
        raise SystemExit(2)
