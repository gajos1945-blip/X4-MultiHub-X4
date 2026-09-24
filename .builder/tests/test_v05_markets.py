from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / ".builder/overlay/src/multihub"

def test_market_sources_exist():
    for name in (
        "MarketStore.h",
        "MarketStore.cpp",
        "MarketGatewayClient.h",
        "MarketGatewayClient.cpp",
        "MarketSearchResultsActivity.h",
        "MarketSearchResultsActivity.cpp",
        "MarketsActivity.h",
        "MarketsActivity.cpp",
    ):
        assert (OVERLAY / name).is_file()

def test_multihub_opens_real_markets():
    source = (OVERLAY / "MultiHubActivity.cpp").read_text(encoding="utf-8")
    assert '#include "MarketsActivity.h"' in source
    assert "std::make_unique<MarketsActivity>" in source
    assert "Markets & Weather - NOT IMPLEMENTED" not in source

def test_market_storage_is_dynamic_and_secret_free():
    header = (OVERLAY / "MarketStore.h").read_text(encoding="utf-8")
    source = (OVERLAY / "MarketStore.cpp").read_text(encoding="utf-8")
    assert 'FAVORITES_PATH = "/.x4-multihub/markets/favorites.jsonl"' in header
    assert 'CONFIG_PATH = "/.x4-multihub/markets/config.json"' in header
    assert "MAX_FAVORITES = 40" in header
    assert "api_token" not in source.lower()
    assert "eodhd" not in source.lower()

def test_market_client_requires_wifi_and_local_http_gateway():
    source = (OVERLAY / "MarketGatewayClient.cpp").read_text(encoding="utf-8")
    assert "WiFi.status() != WL_CONNECTED" in source
    assert 'value.rfind("http://", 0) == 0' in source
    assert "/v1/search?asset=" in source
    assert "/v1/quotes?symbols=" in source
    assert "DATA UNAVAILABLE" not in source

def test_markets_ui_has_dynamic_asset_modes_and_unavailable_state():
    source = (OVERLAY / "MarketsActivity.cpp").read_text(encoding="utf-8")
    for token in (
        "GPW / NewConnect",
        "Crypto",
        "Waluty",
        "Szukaj i dodaj",
        "Odswiez notowania",
        "DATA UNAVAILABLE",
    ):
        assert token in source
    assert 'asset = "crypto"' in source
    assert 'asset = "fx"' in source
    assert 'asset = "pl"' in source

def test_v05_manifest_truthful():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.2-dev"
    assert any("Markets UI" in x for x in data["implemented"])
    assert any("Weather UI" in x for x in data["implemented"])
    assert any("Dashboard combining" in x for x in data["implemented"])
    assert data["physical_device_verified"] is False

def test_release_guard_requires_market_runtime_markers():
    guard = (ROOT / ".builder/tools/release_guard.py").read_text(encoding="utf-8")
    for marker in (
        "GPW / NewConnect / Crypto / FX",
        "X4 Data Gateway",
        "Szukaj i dodaj",
        "Odswiez notowania",
        "DATA UNAVAILABLE",
    ):
        assert marker in guard
