from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / ".builder/overlay/src/multihub"

def test_weather_dashboard_sources_exist():
    for name in (
        "WeatherStore.h", "WeatherStore.cpp",
        "WeatherGatewayClient.h", "WeatherGatewayClient.cpp",
        "WeatherActivity.h", "WeatherActivity.cpp",
        "DashboardStore.h", "DashboardStore.cpp",
        "DashboardSettingsActivity.h", "DashboardSettingsActivity.cpp",
        "DashboardActivity.h", "DashboardActivity.cpp",
    ):
        assert (OVERLAY / name).is_file()

def test_multihub_opens_weather_and_dashboard():
    source = (OVERLAY / "MultiHubActivity.cpp").read_text(encoding="utf-8")
    assert "std::make_unique<WeatherActivity>" in source
    assert "std::make_unique<DashboardActivity>" in source
    assert "Dashboard - NOT IMPLEMENTED" not in source

def test_weather_uses_gateway_not_direct_provider_secret():
    source = (OVERLAY / "WeatherGatewayClient.cpp").read_text(encoding="utf-8")
    assert "/v1/weather?city=" in source
    assert "WiFi.status() != WL_CONNECTED" in source
    assert "api_token" not in source.lower()
    assert "eodhd" not in source.lower()

def test_weather_store_is_bounded():
    source = (OVERLAY / "WeatherStore.cpp").read_text(encoding="utf-8")
    assert "raw.size() < 2048" in source
    assert "file.read()" in source
    assert ".readString()" not in source

def test_dashboard_has_persistent_custom_order_and_visibility():
    header = (OVERLAY / "DashboardStore.h").read_text(encoding="utf-8")
    source = (OVERLAY / "DashboardSettingsActivity.cpp").read_text(encoding="utf-8")
    assert 'CONFIG_PATH = "/.x4-multihub/dashboard/config.json"' in header
    assert "weatherVisible" in header
    assert "marketsVisible" in header
    assert "plannerVisible" in header
    assert "moveUp" in source
    assert "DashboardStore::save(config)" in source

def test_dashboard_uses_only_bounded_market_rows():
    source = (OVERLAY / "DashboardActivity.h").read_text(encoding="utf-8")
    assert "MAX_MARKET_ROWS = 6" in source

def test_v06_manifest_truthful():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.4-dev"
    assert any("Weather UI" in x for x in data["implemented"])
    assert any("Dashboard combining" in x for x in data["implemented"])
    assert any("microSD cache for market quotes" in x for x in data["implemented"])
    assert data["physical_device_verified"] is False

def test_release_guard_requires_weather_dashboard_runtime_markers():
    guard = (ROOT / ".builder/tools/release_guard.py").read_text(encoding="utf-8")
    for marker in (
        "Pogoda / Rynki / Planner",
        "Open-Meteo przez Gateway",
        "Odswiez pogode",
        "Uklad Dashboard",
        "Odswiez Dashboard",
    ):
        assert marker in guard
