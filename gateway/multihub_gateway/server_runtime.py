from __future__ import annotations

from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import secrets
from typing import Callable
import urllib.parse

from .config import GatewayConfig
from .http_client import HttpError
from .providers.eodhd import EodhdProvider, ProviderConfigError
from .providers.open_meteo import OpenMeteoProvider
from .service import MultiHubService


LogCallback = Callable[[str], None]
AUTH_HEADER = "X-X4-Token"


def _build_service(config: GatewayConfig) -> MultiHubService:
    return MultiHubService(
        EodhdProvider(config.eodhd_token),
        OpenMeteoProvider(),
    )


def _authorized(config: GatewayConfig, received: str | None) -> bool:
    expected = config.access_token
    if not expected:
        return True
    if not received:
        return False
    return secrets.compare_digest(received, expected)


def create_http_server(
    config: GatewayConfig,
    log_callback: LogCallback | None = None,
) -> ThreadingHTTPServer:
    service = _build_service(config)

    class Handler(BaseHTTPRequestHandler):
        server_version = "X4MultiHubGateway/1.7"

        def send_json(self, status: int, payload: dict) -> None:
            raw = json.dumps(payload, ensure_ascii=False).encode("utf-8")
            self.send_response(status)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Cache-Control", "no-store")
            self.send_header("Content-Length", str(len(raw)))
            self.end_headers()
            self.wfile.write(raw)

        def log_message(self, fmt: str, *args) -> None:
            if log_callback is not None:
                log_callback(
                    f"{self.client_address[0]} - {fmt % args}"
                )

        def do_GET(self) -> None:
            parsed = urllib.parse.urlsplit(self.path)
            query = urllib.parse.parse_qs(parsed.query)

            try:
                if parsed.path == "/health":
                    self.send_json(
                        200,
                        {
                            "ok": True,
                            "service": "X4 Data Gateway",
                            "version": "1.7",
                            "eodhd_configured": bool(config.eodhd_token),
                            "auth_required": bool(config.access_token),
                            "rss_atom": True,
                        },
                    )
                    return

                if parsed.path.startswith("/v1/"):
                    received = self.headers.get(AUTH_HEADER)
                    if not _authorized(config, received):
                        self.send_json(401, {"error": "unauthorized"})
                        return

                if parsed.path == "/v1/ping":
                    self.send_json(
                        200,
                        {
                            "ok": True,
                            "service": "X4 Data Gateway",
                            "version": "1.7",
                        },
                    )
                    return

                if parsed.path == "/v1/search":
                    asset = query.get("asset", [""])[0]
                    q = query.get("q", [""])[0]
                    self.send_json(200, service.search(asset, q))
                    return

                if parsed.path == "/v1/quote":
                    symbol = query.get("symbol", [""])[0]
                    self.send_json(200, service.quote(symbol))
                    return

                if parsed.path == "/v1/quotes":
                    raw = query.get("symbols", [""])[0]
                    symbols = [x for x in raw.split(",") if x]
                    self.send_json(200, service.quotes(symbols))
                    return

                if parsed.path == "/v1/weather":
                    city = query.get("city", [""])[0]
                    self.send_json(200, service.weather_for_city(city))
                    return

                if parsed.path == "/v1/news":
                    url = query.get("url", [""])[0]
                    raw_limit = query.get("limit", ["15"])[0]
                    try:
                        limit = int(raw_limit)
                    except ValueError as exc:
                        raise ValueError("limit must be an integer") from exc
                    if not (1 <= limit <= 20):
                        raise ValueError("limit must be between 1 and 20")
                    self.send_json(200, service.news(url, limit))
                    return

                self.send_json(404, {"error": "not found"})
            except (ValueError, ProviderConfigError) as exc:
                self.send_json(400, {"error": str(exc)})
            except (HttpError, RuntimeError) as exc:
                self.send_json(502, {"error": str(exc)})
            except Exception:
                self.send_json(500, {"error": "internal gateway error"})

    server = ThreadingHTTPServer((config.host, config.port), Handler)
    server.daemon_threads = True
    return server
