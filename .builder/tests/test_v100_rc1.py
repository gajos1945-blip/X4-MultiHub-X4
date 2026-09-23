from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]

def test_final_version_and_custom_alias_contract():
    source = (ROOT / ".builder/build_ci.py").read_text(encoding="utf-8")
    assert 'RELEASE_NAME = "X4_MultiHub_X4_v1.0.0-rc1.bin"' in source
    assert 'custom_bin = dist / "Custom.bin"' in source
    assert "Custom.bin alias hash differs" in source
    assert '"software_feature_complete": True' in source
    assert '"merged_full_flash": False' in source

def test_final_workflow_uploads_complete_artifact_set():
    workflow = (ROOT / ".github/workflows/build-x4-bin.yml").read_text(encoding="utf-8")
    assert "BUILD X4 MULTIHUB FINAL RC BIN" in workflow
    for filename in (
        "X4_MultiHub_X4_v1.0.0-rc1.bin",
        "X4_MultiHub_X4_v1.0.0-rc1.bin.sha256.txt",
        "Custom.bin",
        "Custom.bin.sha256.txt",
        "build_manifest.json",
        "source_report.json",
        "FINAL_STATUS.txt",
    ):
        assert filename in workflow

def test_final_manifest_remains_hardware_unverified():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.0.0-rc1"
    assert data["software_feature_complete"] is True
    assert data["physical_device_verified"] is False
    assert data["automatic_flash"] is False
    assert data["automatic_erase"] is False
    assert data["merged_full_flash"] is False

def test_about_version_is_final_rc():
    source = (ROOT / ".builder/overlay/src/multihub/MultiHubSettingsActivity.cpp").read_text(encoding="utf-8")
    assert "X4 MultiHub 1.0.0-rc1" in source
    assert "X4 MultiHub 0.9-rc1" not in source
