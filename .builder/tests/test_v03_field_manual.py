from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / ".builder/overlay/src/multihub"

def test_field_manual_sources_exist():
    for name in (
        "FieldManualStore.h",
        "FieldManualStore.cpp",
        "FieldManualActivity.h",
        "FieldManualActivity.cpp",
        "ChecklistActivity.h",
        "ChecklistActivity.cpp",
    ):
        assert (OVERLAY / name).is_file()

def test_multihub_opens_field_manual():
    source = (OVERLAY / "MultiHubActivity.cpp").read_text(encoding="utf-8")
    assert '#include "FieldManualActivity.h"' in source
    assert "std::make_unique<FieldManualActivity>" in source
    assert "Field Manual - NOT IMPLEMENTED" not in source

def test_field_manual_root_is_explicit_and_not_flash_layout_data():
    source = (OVERLAY / "FieldManualStore.h").read_text(encoding="utf-8")
    assert 'MANUALS_ROOT = "/Manuals"' in source
    assert 'INDEX_PATH = "/.x4-multihub/manuals/index.jsonl"' in source

def test_field_manual_has_dynamic_categories_search_favorites_and_checklists():
    activity = (OVERLAY / "FieldManualActivity.cpp").read_text(encoding="utf-8")
    store = (OVERLAY / "FieldManualStore.cpp").read_text(encoding="utf-8")
    checklist = (OVERLAY / "ChecklistActivity.cpp").read_text(encoding="utf-8")
    assert "rebuildCategories()" in activity
    assert "Szukaj: tytul/kategoria/sciezka" in activity
    assert "toggleFavorite" in activity
    assert "Przebuduj indeks" in activity
    assert "isChecklistPath" in store
    assert "parseChecklistLine" in checklist
    assert "saveChecklist()" in checklist

def test_checklist_has_memory_caps():
    header = (OVERLAY / "ChecklistActivity.h").read_text(encoding="utf-8")
    assert "MAX_FILE_BYTES = 16 * 1024" in header
    assert "MAX_ITEMS = 80" in header

def test_v03_manifest_truthful():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "0.6-dev"
    assert any("Field Manual microSD index" in x for x in data["implemented"])
    assert any("Daily Planner" in x for x in data["implemented"])
    assert data["physical_device_verified"] is False

def test_sd_examples_present():
    assert (ROOT / "sdcard/Manuals/README.txt").is_file()
    assert (ROOT / "sdcard/Manuals/Przyklady/Start.txt").is_file()
    sample = (ROOT / "sdcard/Manuals/Przyklady/Kontrola.checklist").read_text(encoding="utf-8")
    assert "[ ]" in sample

def test_release_guard_requires_field_manual_runtime_markers():
    guard = (ROOT / ".builder/tools/release_guard.py").read_text(encoding="utf-8")
    for marker in ("/Manuals", "Przebuduj indeks", "Checklist", "Manuale offline / checklisty"):
        assert marker in guard
