from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / ".builder/overlay/src/multihub"


def test_diagnostics_sources_exist():
    for name in (
        "GatewayDiagnosticsClient.h",
        "GatewayDiagnosticsClient.cpp",
        "DiagnosticsStore.h",
        "DiagnosticsStore.cpp",
        "DiagnosticsActivity.h",
        "DiagnosticsActivity.cpp",
    ):
        assert (OVERLAY / name).is_file()


def test_gateway_diagnostics_tests_health_then_authenticated_ping():
    source = (OVERLAY / "GatewayDiagnosticsClient.cpp").read_text(encoding="utf-8")
    assert 'base + "/health"' in source
    assert 'base + "/v1/ping"' in source
    assert "GatewayAuth::addHeaderIfConfigured(http)" in source
    assert "result.pingStatus == 401" in source
    assert "Gateway token nie pasuje" in source


def test_storage_self_test_is_temporary_and_removed():
    source = (OVERLAY / "DiagnosticsStore.cpp").read_text(encoding="utf-8")
    assert '"/.x4-multihub/diagnostics/.write_test.tmp"' in source
    assert "Storage.writeFile(TEST_PATH" in source
    assert "Storage.openFileForRead" in source
    assert "Storage.remove(TEST_PATH)" in source


def test_report_redacts_gateway_token():
    source = (OVERLAY / "DiagnosticsActivity.cpp").read_text(encoding="utf-8")
    assert "gateway_token_configured=" in source
    assert "gateway_token_value=REDACTED" in source
    assert "GatewayAuthStore::loadToken" not in source


def test_settings_exposes_diagnostics():
    source = (OVERLAY / "MultiHubSettingsActivity.cpp").read_text(encoding="utf-8")
    assert '"Diagnostyka"' in source
    assert "std::make_unique<DiagnosticsActivity>" in source
    assert '"Gateway / microSD / raport"' in source


def test_v17_manifest_truthful():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.7-dev"
    assert data["status"] == "DEVELOPMENT_V1_7_HARDWARE_UNVERIFIED"
    assert data["physical_device_verified"] is False
    assert any("Diagnostics screen" in x for x in data["implemented"])
    assert any("/v1/ping" in x for x in data["implemented"])
    assert any("remote log upload" in x for x in data["not_implemented"])


def test_v17_workflow_and_release_markers():
    workflow = (ROOT / ".github/workflows/build-x4-bin.yml").read_text(encoding="utf-8")
    guard = (ROOT / ".builder/tools/release_guard.py").read_text(encoding="utf-8")
    assert "BUILD X4 MULTIHUB v1.7 DEV BIN" in workflow
    assert "X4_MultiHub_X4_v1_7_dev" in workflow
    assert "X4_Data_Gateway_Windows_v1_7_dev" in workflow
    for marker in (
        "Diagnostyka",
        "Gateway /health",
        "Gateway auth /v1/ping",
        "microSD write test",
        "Raport zapisany",
    ):
        assert marker in guard
