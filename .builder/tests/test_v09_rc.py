from pathlib import Path
import hashlib
import json
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / ".builder/tools"
sys.path.insert(0, str(TOOLS))

from bin_inspector import inspect_bin
from rc_audit import audit_repository


def build_fake_esp_image(path: Path) -> None:
    header = bytearray(24)
    header[0] = 0xE9
    header[1] = 1
    struct.pack_into("<I", header, 4, 0x40380000)
    struct.pack_into("<H", header, 12, 5)
    header[23] = 1

    segment = b"hello rc image"
    body = bytes(header) + struct.pack("<II", 0x3C000020, len(segment)) + segment

    checksum = 0xEF
    for value in segment:
        checksum ^= value

    prehash_end = ((len(body) + 1 + 15) // 16) * 16
    padded = bytearray(body)
    padded.extend(b"\x00" * (prehash_end - len(padded)))
    padded[prehash_end - 1] = checksum

    digest = hashlib.sha256(bytes(padded)).digest()
    path.write_bytes(bytes(padded) + digest)


def test_bin_inspector_validates_checksum_hash_and_tail(tmp_path):
    image = tmp_path / "firmware.bin"
    build_fake_esp_image(image)

    info = inspect_bin(image)
    assert info["esp_image"] is True
    assert info["structure_valid"] is True
    assert info["image_checksum_valid"] is True
    assert info["appended_sha256_valid"] is True
    assert info["trailing_bytes"] == 0
    assert info["chip_id_raw"] == 5


def test_bin_inspector_rejects_corruption(tmp_path):
    image = tmp_path / "firmware.bin"
    build_fake_esp_image(image)
    data = bytearray(image.read_bytes())
    data[32] ^= 0x01
    image.write_bytes(data)

    info = inspect_bin(image)
    assert info["structure_valid"] is False
    assert info["image_checksum_valid"] is False or info["appended_sha256_valid"] is False


def test_rc_source_audit_passes_current_tree():
    assert audit_repository(ROOT) == []


def test_rc_build_manifest_contract():
    source = (ROOT / ".builder/build_ci.py").read_text(encoding="utf-8")
    assert '"version": "1.6-dev"' in source
    assert '"release_status": "DEVELOPMENT_V1_6_HARDWARE_UNVERIFIED"' in source
    assert '"image_integrity_verified": True' in source
    assert '"application_partition_spare_bytes": partition_spare' in source
    assert '"merged_full_flash": False' in source


def test_rc_workflow_and_filename():
    workflow = (ROOT / ".github/workflows/build-x4-bin.yml").read_text(encoding="utf-8")
    assert "BUILD X4 MULTIHUB v1.6 DEV BIN" in workflow
    assert "X4_MultiHub_X4_v1_6_dev" in workflow
    assert "X4_MultiHub_X4_v1.6-dev.bin" in workflow


def test_rc_settings_version_string():
    source = (ROOT / ".builder/overlay/src/multihub/MultiHubSettingsActivity.cpp").read_text(
        encoding="utf-8"
    )
    assert "X4 MultiHub 1.6-dev" in source
    assert "0.8-dev" not in source


def test_manifest_keeps_physical_hardware_unverified():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.6-dev"
    assert data["physical_device_verified"] is False
    assert data["automatic_erase"] is False
    assert data["automatic_flash"] is False
