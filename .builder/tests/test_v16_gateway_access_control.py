from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / ".builder/overlay/src/multihub"


def test_gateway_auth_sources_exist():
    for name in (
        "GatewayAuthStore.h",
        "GatewayAuthStore.cpp",
        "GatewayAuth.h",
        "GatewayAuth.cpp",
    ):
        assert (OVERLAY / name).is_file()


def test_gateway_token_is_runtime_sd_config_not_compiled_secret():
    header = (OVERLAY / "GatewayAuthStore.h").read_text(encoding="utf-8")
    source = (OVERLAY / "GatewayAuthStore.cpp").read_text(encoding="utf-8")
    assert 'CONFIG_PATH = "/.x4-multihub/gateway/auth.json"' in header
    assert 'doc["token"] = token' in source
    assert "MIN_TOKEN_LEN = 8" in header
    assert "MAX_TOKEN_LEN = 96" in header


def test_all_online_clients_add_gateway_auth_header():
    for name in (
        "MarketGatewayClient.cpp",
        "WeatherGatewayClient.cpp",
        "NewsGatewayClient.cpp",
    ):
        source = (OVERLAY / name).read_text(encoding="utf-8")
        assert '#include "GatewayAuth.h"' in source
        assert "GatewayAuth::addHeaderIfConfigured(http);" in source


def test_header_name_is_explicit_and_no_provider_secret_enters_firmware():
    auth = (OVERLAY / "GatewayAuth.cpp").read_text(encoding="utf-8")
    joined = "\n".join(
        p.read_text(encoding="utf-8", errors="replace")
        for p in OVERLAY.glob("*.*")
        if p.suffix in {".cpp", ".h"}
    )
    assert 'http.addHeader("X-X4-Token", token.c_str())' in auth
    assert "EODHD_API_TOKEN" not in joined


def test_settings_never_display_token_value():
    source = (OVERLAY / "MultiHubSettingsActivity.cpp").read_text(encoding="utf-8")
    assert '"Gateway access token"' in source
    assert 'GatewayAuthStore::isConfigured() ? "USTAWIONY" : "BRAK"' in source
    assert '"", GatewayAuthStore::MAX_TOKEN_LEN' in source
    assert "GatewayAuthStore::saveToken(value)" in source
    assert "GatewayAuthStore::clearToken()" in source


def test_v16_manifest_truthful_about_http_limitation():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.6-dev"
    assert data["status"] == "DEVELOPMENT_V1_6_HARDWARE_UNVERIFIED"
    assert data["physical_device_verified"] is False
    assert any("shared-token access control" in x for x in data["implemented"])
    assert any("TLS encryption" in x for x in data["not_implemented"])


def test_v16_workflow_and_release_guard():
    workflow = (ROOT / ".github/workflows/build-x4-bin.yml").read_text(encoding="utf-8")
    guard = (ROOT / ".builder/tools/release_guard.py").read_text(encoding="utf-8")
    assert "BUILD X4 MULTIHUB v1.6 DEV BIN" in workflow
    assert "X4_MultiHub_X4_v1_6_dev" in workflow
    assert "X4_Data_Gateway_Windows_v1_6_dev" in workflow
    for marker in (
        "Gateway access token",
        "Gateway token zapisany",
        "Gateway token usuniety",
    ):
        assert marker in guard
