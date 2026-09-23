from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]

def test_reader_entry_is_real_navigation_not_placeholder():
    source = (ROOT / ".builder/overlay/src/multihub/MultiHubActivity.cpp").read_text(encoding="utf-8")
    assert "activityManager.goToFileBrowser();" in source
    assert 'values[0] = "Otworz biblioteke / pliki";' in source
    assert "Reader: uzyj Browse Files na Home" not in source

def test_activity_manager_include_present():
    source = (ROOT / ".builder/overlay/src/multihub/MultiHubActivity.cpp").read_text(encoding="utf-8")
    assert '#include "activities/ActivityManager.h"' in source

def test_v02_manifest_truthful():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "0.7-dev"
    assert any("Reader entry routes" in x for x in data["implemented"])
    assert any("Field Manual" in x for x in data["implemented"])
    assert data["physical_device_verified"] is False

def test_v02_workflow_artifact_name():
    workflow = (ROOT / ".github/workflows/build-x4-bin.yml").read_text(encoding="utf-8")
    assert "X4_MultiHub_X4_v0_7_dev" in workflow
    assert "X4_MultiHub_X4_v0.7-dev.bin" in workflow
