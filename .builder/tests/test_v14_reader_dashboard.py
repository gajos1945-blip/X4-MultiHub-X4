from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / ".builder/overlay/src/multihub"


def test_reader_dashboard_sources_exist():
    for name in (
        "ReaderDashboardStore.h", "ReaderDashboardStore.cpp",
        "ReaderDashboardActivity.h", "ReaderDashboardActivity.cpp",
    ):
        assert (OVERLAY / name).is_file()


def test_reader_dashboard_uses_crosspoint_state_and_recent_store():
    source = (OVERLAY / "ReaderDashboardStore.cpp").read_text(encoding="utf-8")
    assert "APP_STATE.openEpubPath" in source
    assert "RECENT_BOOKS.getBooks()" in source
    assert "Storage.exists(snapshot.path.c_str())" in source


def test_reader_dashboard_reads_progress_bin_read_only():
    source = (OVERLAY / "ReaderDashboardStore.cpp").read_text(encoding="utf-8")
    assert '"/progress.bin"' in source
    assert "Storage.openFileForRead" in source
    assert "openFileForWrite" not in source
    assert "Storage.writeFile" not in source


def test_epub_cache_formula_matches_pinned_crosspoint_pattern():
    source = (OVERLAY / "ReaderDashboardStore.cpp").read_text(encoding="utf-8")
    assert '"/.crosspoint/epub_"' in source
    assert "std::hash<std::string>{}(path)" in source


def test_book_percentage_uses_lightweight_cached_epub_load():
    source = (OVERLAY / "ReaderDashboardStore.cpp").read_text(encoding="utf-8")
    assert 'Epub epub(snapshot.path, "/.crosspoint")' in source
    assert "epub.load(false, true)" in source
    assert "epub.calculateProgress" in source


def test_dashboard_has_reader_card_and_continue_activity():
    dashboard = (OVERLAY / "DashboardActivity.cpp").read_text(encoding="utf-8")
    activity = (OVERLAY / "ReaderDashboardActivity.cpp").read_text(encoding="utf-8")
    assert 'card == "reader"' in dashboard
    assert "ReaderDashboardStore::bookProgressLabel" in dashboard
    assert "std::make_unique<ReaderDashboardActivity>" in dashboard
    assert "activityManager.goToReader(snapshot.path)" in activity
    assert "activityManager.goToFileBrowser()" in activity


def test_dashboard_config_migrates_existing_layouts_with_reader_card():
    source = (OVERLAY / "DashboardStore.cpp").read_text(encoding="utf-8")
    assert 'config.order = {"reader", "weather", "markets", "planner"}' in source
    assert 'order.insert(order.begin(), "reader")' in source
    assert 'doc["reader_visible"]' in source


def test_v14_manifest_truthful():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.4-dev"
    assert data["status"] == "DEVELOPMENT_V1_4_HARDWARE_UNVERIFIED"
    assert data["physical_device_verified"] is False
    assert any("Reader Dashboard card" in x for x in data["implemented"])
    assert any("non-EPUB" in x for x in data["not_implemented"])


def test_v14_workflow_and_guard():
    workflow = (ROOT / ".github/workflows/build-x4-bin.yml").read_text(encoding="utf-8")
    guard = (ROOT / ".builder/tools/release_guard.py").read_text(encoding="utf-8")
    assert "BUILD X4 MULTIHUB v1.4 DEV BIN" in workflow
    assert "X4_MultiHub_X4_v1_4_dev" in workflow
    assert "X4_MultiHub_X4_v1.4-dev.bin" in workflow
    for marker in (
        "Reader Dashboard",
        "Kontynuuj czytanie",
        "Potwierdzony z cache CrossPoint",
        "Postep calej ksiazki UNKNOWN",
        "Brak ostatniej ksiazki",
    ):
        assert marker in guard
