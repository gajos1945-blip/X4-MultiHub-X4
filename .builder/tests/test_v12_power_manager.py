from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / ".builder/overlay/src/multihub"

def test_power_sources_exist():
    for name in (
        "PowerStore.h", "PowerStore.cpp",
        "PowerManager.h", "PowerManager.cpp",
        "PowerSettingsActivity.h", "PowerSettingsActivity.cpp",
    ):
        assert (OVERLAY / name).is_file()

def test_power_defaults_are_conservative():
    header = (OVERLAY / "PowerStore.h").read_text(encoding="utf-8")
    assert "autoRefreshOnOpen = false" in header
    assert "radioOffAfterRefresh = false" in header

def test_wifi_off_does_not_intentionally_erase_ap_config():
    source = (OVERLAY / "PowerManager.cpp").read_text(encoding="utf-8")
    assert "WiFi.disconnect(true, false)" in source
    assert "WiFi.disconnect(true, true)" not in source

def test_news_manual_refresh_by_default_uses_power_policy():
    source = (OVERLAY / "NewsFeedActivity.cpp").read_text(encoding="utf-8")
    assert "PowerManager::autoRefreshOnOpen()" in source
    assert "PowerManager::afterOnlineOperation()" in source

def test_weather_markets_dashboard_have_power_hooks():
    weather = (OVERLAY / "WeatherActivity.cpp").read_text(encoding="utf-8")
    markets = (OVERLAY / "MarketsActivity.cpp").read_text(encoding="utf-8")
    dashboard = (OVERLAY / "DashboardActivity.cpp").read_text(encoding="utf-8")
    assert "PowerManager::afterOnlineOperation()" in weather
    assert markets.count("PowerManager::afterOnlineOperation()") >= 2
    assert "onlineAttempted" in dashboard
    assert "PowerManager::afterOnlineOperation()" in dashboard

def test_settings_exposes_power_manager():
    source = (OVERLAY / "MultiHubSettingsActivity.cpp").read_text(encoding="utf-8")
    assert '"Power Manager"' in source
    assert "std::make_unique<PowerSettingsActivity>" in source
    assert '"Siec / odswiezanie / Wi-Fi OFF"' in source

def test_v12_manifest_truthful_about_hardware_limits():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.7-dev"
    assert data["status"] == "DEVELOPMENT_V1_7_HARDWARE_UNVERIFIED"
    assert data["physical_device_verified"] is False
    assert any("deep sleep" in item.lower() for item in data["not_implemented"])
    assert any("battery" in item.lower() for item in data["not_implemented"])

def test_v12_workflow_names():
    workflow = (ROOT / ".github/workflows/build-x4-bin.yml").read_text(encoding="utf-8")
    assert "BUILD X4 MULTIHUB v1.7 DEV BIN" in workflow
    assert "X4_MultiHub_X4_v1_7_dev" in workflow
    assert "X4_MultiHub_X4_v1.7-dev.bin" in workflow

def test_release_guard_requires_power_markers():
    guard = (ROOT / ".builder/tools/release_guard.py").read_text(encoding="utf-8")
    for marker in (
        "Power Manager",
        "Online przy otwarciu",
        "Wi-Fi OFF po odswiezeniu",
        "Wylacz Wi-Fi teraz",
    ):
        assert marker in guard
