from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / ".builder/overlay/src/multihub"

def test_cache_sources_exist():
    assert (OVERLAY / "DataCache.h").is_file()
    assert (OVERLAY / "DataCache.cpp").is_file()

def test_cache_paths_are_on_multihub_sd_root():
    header = (OVERLAY / "DataCache.h").read_text(encoding="utf-8")
    assert 'MARKET_QUOTES_PATH = "/.x4-multihub/cache/market_quotes.json"' in header
    assert 'WEATHER_PATH = "/.x4-multihub/cache/weather.json"' in header

def test_cache_writes_are_bounded_and_atomic():
    source = (OVERLAY / "DataCache.cpp").read_text(encoding="utf-8")
    assert "MAX_CACHE_BYTES = 24 * 1024" in source
    assert 'std::string(path) + ".tmp"' in source
    assert "Storage.rename" in source
    assert "raw.size() < MAX_CACHE_BYTES" in source

def test_markets_weather_dashboard_have_cache_fallback():
    markets = (OVERLAY / "MarketsActivity.cpp").read_text(encoding="utf-8")
    weather = (OVERLAY / "WeatherActivity.cpp").read_text(encoding="utf-8")
    dashboard = (OVERLAY / "DashboardActivity.cpp").read_text(encoding="utf-8")
    assert "DataCache::loadMarketQuotes" in markets
    assert "DataCache::saveMarketQuotes" in markets
    assert "Markets CACHED" in markets
    assert "DataCache::loadWeather" in weather
    assert "DataCache::saveWeather" in weather
    assert "Pogoda CACHED" in weather
    assert "DataCache::loadMarketQuotes" in dashboard
    assert "DataCache::loadWeather" in dashboard
    assert "Dashboard CACHED" in dashboard

def test_gateway_retry_backoff_is_bounded():
    source = (ROOT / "gateway/multihub_gateway/http_client.py").read_text(encoding="utf-8")
    assert "RETRYABLE_HTTP = {429, 500, 502, 503, 504}" in source
    assert "attempts = max(1, min(int(attempts), 4))" in source
    assert "time.sleep" in source

def test_v07_manifest_truthful():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.4-dev"
    assert any("microSD cache for market quotes" in x for x in data["implemented"])
    assert any("retry/backoff" in x for x in data["implemented"])
    assert any("cache age" in x.lower() for x in data["implemented"])
    assert any("RTC" in x for x in data["not_implemented"])
    assert data["physical_device_verified"] is False

def test_release_guard_requires_cache_markers():
    guard = (ROOT / ".builder/tools/release_guard.py").read_text(encoding="utf-8")
    for marker in ("Markets CACHED", "Pogoda CACHED", "Dashboard CACHED", "CACHED:"):
        assert marker in guard
