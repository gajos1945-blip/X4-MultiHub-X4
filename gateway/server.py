from __future__ import annotations

from multihub_gateway.config import GatewayConfig
from multihub_gateway.server_runtime import create_http_server


def main() -> None:
    config = GatewayConfig.from_env()
    server = create_http_server(config, print)
    print(f"X4 Data Gateway listening on http://{config.host}:{config.port}")
    print(f"EODHD configured: {'YES' if config.eodhd_token else 'NO'}")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
