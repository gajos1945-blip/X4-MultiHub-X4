from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / ".builder/overlay/src/multihub"

def test_settings_sources_exist():
    assert (OVERLAY / "MultiHubSettingsActivity.h").is_file()
    assert (OVERLAY / "MultiHubSettingsActivity.cpp").is_file()

def test_multihub_settings_is_real_not_placeholder():
    source = (OVERLAY / "MultiHubActivity.cpp").read_text(encoding="utf-8")
    assert '#include "MultiHubSettingsActivity.h"' in source
    assert "std::make_unique<MultiHubSettingsActivity>" in source
    assert "NOT IMPLEMENTED v0.7" not in source

def test_main_menu_polish_oriented_labels():
    source = (OVERLAY / "MultiHubActivity.cpp").read_text(encoding="utf-8")
    for label in ("Czytnik", "Instrukcje", "Planer", "Rynki", "Pogoda", "Dashboard", "Ustawienia"):
        assert f'"{label}"' in source

def test_settings_centralizes_required_controls():
    source = (OVERLAY / "MultiHubSettingsActivity.cpp").read_text(encoding="utf-8")
    for marker in (
        "X4 Data Gateway",
        "Miasto pogody",
        "Data planera",
        "Uklad Dashboard",
        "Wyczysc cache rynkow",
        "Wyczysc cache pogody",
        "Wyczysc caly cache",
        "X4 MultiHub 1.6-dev",
    ):
        assert marker in source

def test_settings_reuses_verified_storage_layers():
    source = (OVERLAY / "MultiHubSettingsActivity.cpp").read_text(encoding="utf-8")
    assert "MarketStore::saveGateway" in source
    assert "WeatherStore::saveCity" in source
    assert "PlannerStore::saveActiveDate" in source
    assert "DataCache::clearMarkets" in source
    assert "DataCache::clearWeather" in source
    assert "DashboardStore::load" in source

def test_v08_manifest_truthful():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.6-dev"
    assert any("Central MultiHub Settings" in x for x in data["implemented"])
    assert any("Planner Today" in x for x in data["implemented"])
    assert "Full Polish diacritics validation on physical X4 font/rendering stack" in data["not_implemented"]
    assert data["physical_device_verified"] is False

def test_release_guard_no_longer_requires_settings_placeholder():
    guard = (ROOT / ".builder/tools/release_guard.py").read_text(encoding="utf-8")
    assert "NOT IMPLEMENTED v0.7" not in guard
    for marker in (
        "Ustawienia MultiHub",
        "Wyczysc cache rynkow",
        "Wyczysc cache pogody",
        "Wyczysc caly cache",
        "X4 MultiHub 1.6-dev",
    ):
        assert marker in guard
