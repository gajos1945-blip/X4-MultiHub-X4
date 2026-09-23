from __future__ import annotations
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
from pathlib import Path
import sys
import urllib.parse

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT))

from multihub_gateway.config import GatewayConfig
from multihub_gateway.http_client import HttpError
from multihub_gateway.providers.eodhd import EodhdProvider, ProviderConfigError
from multihub_gateway.providers.open_meteo import OpenMeteoProvider
from multihub_gateway.service import MultiHubService

CONFIG = GatewayConfig.from_env()
SERVICE = MultiHubService(
    EodhdProvider(CONFIG.eodhd_token),
    OpenMeteoProvider(),
)

class Handler(BaseHTTPRequestHandler):
    server_version = "X4MultiHubGateway/0.1"

    def send_json(self, status: int, payload: dict) -> None:
        raw = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(raw)))
        self.end_headers()
        self.wfile.write(raw)

    def do_GET(self) -> None:
        parsed = urllib.parse.urlsplit(self.path)
        query = urllib.parse.parse_qs(parsed.query)

        try:
            if parsed.path == "/health":
                self.send_json(200, {
                    "ok": True,
                    "service": "X4 Data Gateway",
                    "version": "0.1",
                    "eodhd_configured": bool(CONFIG.eodhd_token),
                })
                return

            if parsed.path == "/v1/search":
                asset = query.get("asset", [""])[0]
                q = query.get("q", [""])[0]
                self.send_json(200, SERVICE.search(asset, q))
                return

            if parsed.path == "/v1/quote":
                symbol = query.get("symbol", [""])[0]
                self.send_json(200, SERVICE.quote(symbol))
                return

            if parsed.path == "/v1/weather":
                city = query.get("city", [""])[0]
                self.send_json(200, SERVICE.weather_for_city(city))
                return

            self.send_json(404, {"error": "not found"})
        except (ValueError, ProviderConfigError) as exc:
            self.send_json(400, {"error": str(exc)})
        except (HttpError, RuntimeError) as exc:
            self.send_json(502, {"error": str(exc)})
        except Exception:
            self.send_json(500, {"error": "internal gateway error"})

def main() -> None:
    server = ThreadingHTTPServer((CONFIG.host, CONFIG.port), Handler)
    print(f"X4 Data Gateway listening on http://{CONFIG.host}:{CONFIG.port}")
    print(f"EODHD configured: {'YES' if CONFIG.eodhd_token else 'NO'}")
    server.serve_forever()

if __name__ == "__main__":
    main()
