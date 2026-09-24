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


def _start(token: str):
    server = create_http_server(
        GatewayConfig(
            host="127.0.0.1",
            port=0,
            eodhd_token="",
            access_token=token,
        )
    )
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    return server, thread


def _request(url: str, token: str | None = None):
    req = urllib.request.Request(url)
    if token:
        req.add_header("X-X4-Token", token)
    with urllib.request.urlopen(req, timeout=3.0) as response:
        return response.status, json.loads(response.read().decode("utf-8"))


def test_ping_is_local_and_authenticated():
    server, thread = _start("shared-secret")
    try:
        port = server.server_address[1]
        url = f"http://127.0.0.1:{port}/v1/ping"

        try:
            _request(url)
            assert False, "missing token must be rejected"
        except urllib.error.HTTPError as exc:
            assert exc.code == 401

        status, body = _request(url, "shared-secret")
        assert status == 200
        assert body == {
            "ok": True,
            "service": "X4 Data Gateway",
            "version": "1.7",
        }
    finally:
        server.shutdown()
        server.server_close()
        thread.join(timeout=3.0)


def test_ping_works_when_auth_is_disabled():
    server, thread = _start("")
    try:
        port = server.server_address[1]
        status, body = _request(f"http://127.0.0.1:{port}/v1/ping")
        assert status == 200
        assert body["ok"] is True
        assert body["version"] == "1.7"
    finally:
        server.shutdown()
        server.server_close()
        thread.join(timeout=3.0)
