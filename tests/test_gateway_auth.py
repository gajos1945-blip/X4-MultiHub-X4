from __future__ import annotations

import json
from pathlib import Path
import sys
import threading
import urllib.error
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "gateway"))

from multihub_gateway.config import GatewayConfig
from multihub_gateway.server_runtime import create_http_server


def _start(config: GatewayConfig):
    server = create_http_server(config)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    return server, thread


def _json(url: str, token: str | None = None):
    req = urllib.request.Request(url)
    if token:
        req.add_header("X-X4-Token", token)
    with urllib.request.urlopen(req, timeout=3.0) as response:
        return response.status, json.loads(response.read().decode("utf-8"))


def test_health_remains_public_but_reports_auth_required():
    server, thread = _start(
        GatewayConfig(
            host="127.0.0.1",
            port=0,
            eodhd_token="",
            access_token="shared-secret",
        )
    )
    try:
        port = server.server_address[1]
        status, body = _json(f"http://127.0.0.1:{port}/health")
        assert status == 200
        assert body["ok"] is True
        assert body["auth_required"] is True
        assert "shared-secret" not in json.dumps(body)
    finally:
        server.shutdown()
        server.server_close()
        thread.join(timeout=3.0)


def test_v1_route_rejects_missing_and_wrong_token_before_provider_call():
    server, thread = _start(
        GatewayConfig(
            host="127.0.0.1",
            port=0,
            eodhd_token="",
            access_token="shared-secret",
        )
    )
    try:
        port = server.server_address[1]
        url = f"http://127.0.0.1:{port}/v1/weather?city=Katowice"

        for token in (None, "wrong-secret"):
            req = urllib.request.Request(url)
            if token:
                req.add_header("X-X4-Token", token)
            try:
                urllib.request.urlopen(req, timeout=3.0)
                assert False, "request should be rejected"
            except urllib.error.HTTPError as exc:
                assert exc.code == 401
                body = json.loads(exc.read().decode("utf-8"))
                assert body == {"error": "unauthorized"}
    finally:
        server.shutdown()
        server.server_close()
        thread.join(timeout=3.0)


def test_auth_disabled_preserves_backward_compatibility():
    server, thread = _start(
        GatewayConfig(
            host="127.0.0.1",
            port=0,
            eodhd_token="",
            access_token="",
        )
    )
    try:
        port = server.server_address[1]
        status, body = _json(f"http://127.0.0.1:{port}/health")
        assert status == 200
        assert body["auth_required"] is False
    finally:
        server.shutdown()
        server.server_close()
        thread.join(timeout=3.0)
