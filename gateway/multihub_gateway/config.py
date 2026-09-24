from __future__ import annotations
from dataclasses import dataclass
import os


@dataclass(frozen=True)
class GatewayConfig:
    host: str
    port: int
    eodhd_token: str
    access_token: str = ""

    @classmethod
    def from_env(cls) -> "GatewayConfig":
        port = int(os.environ.get("MULTIHUB_GATEWAY_PORT", "8788"))
        if not (1 <= port <= 65535):
            raise ValueError("MULTIHUB_GATEWAY_PORT out of range")
        return cls(
            host=os.environ.get("MULTIHUB_GATEWAY_HOST", "0.0.0.0"),
            port=port,
            eodhd_token=os.environ.get("EODHD_API_TOKEN", "").strip(),
            access_token=os.environ.get("MULTIHUB_GATEWAY_TOKEN", "").strip(),
        )
