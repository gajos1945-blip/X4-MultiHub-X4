from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / ".builder/overlay/src/multihub"


def test_time_sources_exist():
    for name in (
        "TimeStore.h", "TimeStore.cpp",
        "TimeService.h", "TimeService.cpp",
        "TimeSettingsActivity.h", "TimeSettingsActivity.cpp",
    ):
        assert (OVERLAY / name).is_file()


def test_time_defaults_are_conservative():
    header = (OVERLAY / "TimeStore.h").read_text(encoding="utf-8")
    assert "timezonePreset = 0" in header
    assert "autoToday = false" in header


def test_timezone_presets_are_explicit_posix_rules():
    source = (OVERLAY / "TimeStore.cpp").read_text(encoding="utf-8")
    assert '{"UTC", "UTC0"}' in source
    assert '"CET-1CEST,M3.5.0,M10.5.0/3"' in source
    assert '"GMT0BST,M3.5.0/1,M10.5.0"' in source
    assert '"EST5EDT,M3.2.0/2,M11.1.0/2"' in source


def test_sntp_requires_existing_wifi_and_does_not_connect_itself():
    source = (OVERLAY / "TimeService.cpp").read_text(encoding="utf-8")
    assert "WiFi.status() != WL_CONNECTED" in source
    assert "WiFi.begin(" not in source
    assert "WiFi.disconnect(" not in source


def test_sntp_reuses_or_starts_espidf_service_and_requires_fresh_completion():
    source = (OVERLAY / "TimeService.cpp").read_text(encoding="utf-8")
    assert "esp_sntp_enabled()" in source
    assert "esp_sntp_restart()" in source
    assert "esp_sntp_init()" in source
    assert "esp_sntp_set_sync_status(SNTP_SYNC_STATUS_RESET)" in source
    assert "esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED" in source
    assert '"pool.ntp.org"' in source
    assert '"time.google.com"' in source


def test_auto_today_is_opt_in_and_valid_time_guarded():
    source = (OVERLAY / "TimeService.cpp").read_text(encoding="utf-8")
    assert "autoTodayEnabled()" in source
    assert "if (!autoTodayEnabled()) return false;" in source
    assert "if (!today(date)) return false;" in source
    assert "PlannerStore::saveActiveDate(date)" in source


def test_planner_and_dashboard_apply_auto_today():
    planner = (OVERLAY / "DailyPlannerActivity.cpp").read_text(encoding="utf-8")
    dashboard = (OVERLAY / "DashboardActivity.cpp").read_text(encoding="utf-8")
    assert "TimeService::applyTodayToPlanner" in planner
    assert "TimeService::applyTodayToPlanner" in dashboard


def test_cache_timestamp_is_backward_compatible_and_optional():
    header = (OVERLAY / "DataCache.h").read_text(encoding="utf-8")
    source = (OVERLAY / "DataCache.cpp").read_text(encoding="utf-8")
    assert "int64_t* cachedAtEpoch = nullptr" in header
    assert 'doc["cached_at_epoch"] = TimeService::nowEpoch();' in source
    assert 'doc["cached_at_epoch"] | 0LL' in source


def test_cache_age_is_visible_in_markets_weather_dashboard():
    markets = (OVERLAY / "MarketsActivity.cpp").read_text(encoding="utf-8")
    weather = (OVERLAY / "WeatherActivity.cpp").read_text(encoding="utf-8")
    dashboard = (OVERLAY / "DashboardActivity.cpp").read_text(encoding="utf-8")
    assert "TimeService::cacheAgeLabel(cacheEpoch)" in markets
    assert "TimeService::cacheAgeLabel(cacheEpoch)" in weather
    assert "TimeService::cacheAgeLabel(weatherCacheEpoch)" in dashboard
    assert "TimeService::cacheAgeLabel(marketsCacheEpoch)" in dashboard


def test_multihub_settings_exposes_time_screen():
    source = (OVERLAY / "MultiHubSettingsActivity.cpp").read_text(encoding="utf-8")
    assert '"Czas / NTP / Today"' in source
    assert "std::make_unique<TimeSettingsActivity>" in source
    assert '"X4 MultiHub 1.7-dev"' in source


def test_v13_manifest_is_truthful():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.7-dev"
    assert data["status"] == "DEVELOPMENT_V1_7_HARDWARE_UNVERIFIED"
    assert data["physical_device_verified"] is False
    assert data["software_feature_complete"] is False
    assert any("SNTP" in x for x in data["implemented"])
    assert any("RTC" in x for x in data["not_implemented"])


def test_v13_workflow_and_release_guard():
    workflow = (ROOT / ".github/workflows/build-x4-bin.yml").read_text(encoding="utf-8")
    guard = (ROOT / ".builder/tools/release_guard.py").read_text(encoding="utf-8")
    assert "BUILD X4 MULTIHUB v1.7 DEV BIN" in workflow
    assert "X4_MultiHub_X4_v1_7_dev" in workflow
    assert "X4_MultiHub_X4_v1.7-dev.bin" in workflow
    for marker in (
        "Czas / NTP / Today",
        "Planner: automatyczne Today",
        "Synchronizuj NTP",
        "Strefa czasowa",
        "NTP OK",
    ):
        assert marker in guard
