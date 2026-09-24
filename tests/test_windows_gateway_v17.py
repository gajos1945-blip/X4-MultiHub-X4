from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def test_windows_gateway_has_x4_auth_test():
    source = (ROOT / "gateway/windows_app.py").read_text(encoding="utf-8")
    assert 'text="Test X4 auth"' in source
    assert '"/v1/ping"' in source
    assert 'req.add_header("X-X4-Token", token)' in source
    assert "PING " in source


def test_windows_gateway_does_not_log_access_token_value():
    source = (ROOT / "gateway/windows_app.py").read_text(encoding="utf-8")
    assert '"AUTH: "' in source
    assert '"REQUIRED" if access_token else "DISABLED"' in source
    assert 'self._log(access_token)' not in source
