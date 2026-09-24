from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]

def test_v11_keeps_strong_image_validation_from_final_rc():
    source = (ROOT / ".builder/build_ci.py").read_text(encoding="utf-8")
    assert 'RELEASE_NAME = "X4_MultiHub_X4_v1.1-dev.bin"' in source
    assert "structure_valid" in source
    assert "image_checksum_valid" in source
    assert "appended_sha256_valid" in source
    assert '"merged_full_flash": False' in source

def test_v11_is_a_development_artifact_not_custom_release_alias():
    workflow = (ROOT / ".github/workflows/build-x4-bin.yml").read_text(encoding="utf-8")
    assert "X4_MultiHub_X4_v1.1-dev.bin" in workflow
    assert "Custom.bin" not in workflow
    assert "DEV_STATUS.txt" in workflow

def test_v11_manifest_remains_hardware_unverified():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.1-dev"
    assert data["software_feature_complete"] is False
    assert data["physical_device_verified"] is False
    assert data["automatic_flash"] is False
    assert data["automatic_erase"] is False
    assert data["merged_full_flash"] is False

def test_about_version_is_v11_dev():
    source = (ROOT / ".builder/overlay/src/multihub/MultiHubSettingsActivity.cpp").read_text(encoding="utf-8")
    assert "X4 MultiHub 1.1-dev" in source
