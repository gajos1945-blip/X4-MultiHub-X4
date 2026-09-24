from __future__ import annotations

from pathlib import Path
import json
import sys
import threading
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "gateway"))

from multihub_gateway.config import GatewayConfig
from multihub_gateway.server_runtime import create_http_server
from multihub_gateway.windows_config import (
    WindowsGatewaySettings,
    load_settings,
    save_settings,
    validate_port,
)


def test_windows_settings_roundtrip_keeps_token_out_of_plaintext(tmp_path):
    path = tmp_path / "gateway.json"

    def encrypt(value: str) -> str:
        return "ENC:" + value[::-1]

    def decrypt(value: str) -> str:
        assert value.startswith("ENC:")
        return value[4:][::-1]

    original = WindowsGatewaySettings(port=9876, eodhd_token="super-secret-token", access_token="x4-access-secret")
    save_settings(original, path, encrypt=encrypt)

    raw = path.read_text(encoding="utf-8")
    assert "super-secret-token" not in raw
    assert "x4-access-secret" not in raw
    assert '"port": 9876' in raw

    loaded = load_settings(path, decrypt=decrypt)
    assert loaded.port == 9876
    assert loaded.eodhd_token == "super-secret-token"
    assert loaded.access_token == "x4-access-secret"


def test_windows_settings_missing_file_uses_safe_defaults(tmp_path):
    loaded = load_settings(tmp_path / "missing.json", decrypt=lambda value: value)
    assert loaded.port == 8788
    assert loaded.eodhd_token == ""
    assert loaded.access_token == ""


def test_gateway_port_validation():
    assert validate_port(8788) == 8788
    for value in (0, 65536, -1):
        try:
            validate_port(value)
            assert False, f"should reject {value}"
        except ValueError:
            pass


def test_reusable_gateway_runtime_health():
    server = create_http_server(
        GatewayConfig(host="127.0.0.1", port=0, eodhd_token="")
    )
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()

    try:
        port = server.server_address[1]
        with urllib.request.urlopen(
            f"http://127.0.0.1:{port}/health", timeout=3.0
        ) as response:
            body = json.loads(response.read().decode("utf-8"))

        assert response.status == 200
        assert body["ok"] is True
        assert body["service"] == "X4 Data Gateway"
        assert body["version"] == "1.6"
        assert body["eodhd_configured"] is False
        assert body["rss_atom"] is True
    finally:
        server.shutdown()
        server.server_close()
        thread.join(timeout=3.0)


def test_windows_app_does_not_embed_provider_token():
    source = (ROOT / "gateway/windows_app.py").read_text(encoding="utf-8")
    assert "EODHD_API_TOKEN" not in source
    assert "save_settings(settings)" in source
    assert 'host="0.0.0.0"' in source


def test_dpapi_module_uses_windows_cryptprotectdata():
    source = (
        ROOT / "gateway/multihub_gateway/windows_secret.py"
    ).read_text(encoding="utf-8")
    assert "CryptProtectData" in source
    assert "CryptUnprotectData" in source
    assert "base64.b64encode" in source


def test_windows_pyinstaller_build_is_in_workflow():
    workflow = (
        ROOT / ".github/workflows/build-x4-bin.yml"
    ).read_text(encoding="utf-8")
    assert "runs-on: windows-latest" in workflow
    assert "pyinstaller --noconfirm --clean X4DataGateway.spec" in workflow
    assert "X4_Data_Gateway_Windows_v1_6_dev" in workflow
    assert "gateway/dist/X4DataGateway.exe" in workflow


def test_gateway_spec_is_windowed():
    spec = (ROOT / "gateway/X4DataGateway.spec").read_text(encoding="utf-8")
    assert 'name="X4DataGateway"' in spec
    assert "console=False" in spec
