from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]


def test_v15_manifest_truthful():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.5-dev"
    assert data["status"] == "DEVELOPMENT_V1_5_HARDWARE_UNVERIFIED"
    assert data["physical_device_verified"] is False
    assert any("Windows X4 Data Gateway GUI" in x for x in data["implemented"])
    assert any("DPAPI" in x for x in data["implemented"])
    assert any("Gateway authentication" in x for x in data["not_implemented"])


def test_v15_firmware_workflow_name_and_bin():
    workflow = (ROOT / ".github/workflows/build-x4-bin.yml").read_text(encoding="utf-8")
    assert "BUILD X4 MULTIHUB v1.5 DEV BIN" in workflow
    assert "X4_MultiHub_X4_v1_5_dev" in workflow
    assert "X4_MultiHub_X4_v1.5-dev.bin" in workflow


def test_v15_about_marker():
    source = (
        ROOT / ".builder/overlay/src/multihub/MultiHubSettingsActivity.cpp"
    ).read_text(encoding="utf-8")
    assert "X4 MultiHub 1.5-dev" in source


def test_gateway_docs_warn_against_internet_exposure():
    readme = (ROOT / "gateway/README.md").read_text(encoding="utf-8")
    windows = (ROOT / "gateway/WINDOWS_GATEWAY_README.txt").read_text(encoding="utf-8")
    assert "Do not port-forward it directly to the Internet" in readme
    assert "nie wystawiaj portu gateway bezposrednio do Internetu" in windows
