from __future__ import annotations

from dataclasses import dataclass
import json
import os
from pathlib import Path
from typing import Callable

from .windows_secret import protect_text, unprotect_text


@dataclass
class WindowsGatewaySettings:
    port: int = 8788
    eodhd_token: str = ""
    access_token: str = ""


def default_settings_path() -> Path:
    base = os.environ.get("LOCALAPPDATA")
    if base:
        return Path(base) / "X4MultiHub" / "gateway.json"
    return Path.home() / ".x4-multihub" / "gateway.json"


def validate_port(port: int) -> int:
    value = int(port)
    if not (1 <= value <= 65535):
        raise ValueError("port must be between 1 and 65535")
    return value


def load_settings(
    path: Path | None = None,
    *,
    decrypt: Callable[[str], str] = unprotect_text,
) -> WindowsGatewaySettings:
    target = path or default_settings_path()
    if not target.exists():
        return WindowsGatewaySettings()

    data = json.loads(target.read_text(encoding="utf-8"))
    settings = WindowsGatewaySettings(
        port=validate_port(data.get("port", 8788)),
        eodhd_token="",
        access_token="",
    )

    encrypted_eodhd = str(data.get("eodhd_token_dpapi", "") or "")
    if encrypted_eodhd:
        settings.eodhd_token = decrypt(encrypted_eodhd)

    encrypted_access = str(data.get("access_token_dpapi", "") or "")
    if encrypted_access:
        settings.access_token = decrypt(encrypted_access)

    return settings


def save_settings(
    settings: WindowsGatewaySettings,
    path: Path | None = None,
    *,
    encrypt: Callable[[str], str] = protect_text,
) -> Path:
    target = path or default_settings_path()
    target.parent.mkdir(parents=True, exist_ok=True)

    payload = {
        "version": 2,
        "port": validate_port(settings.port),
        "eodhd_token_dpapi": encrypt(settings.eodhd_token)
        if settings.eodhd_token
        else "",
        "access_token_dpapi": encrypt(settings.access_token)
        if settings.access_token
        else "",
    }

    temp = target.with_suffix(target.suffix + ".tmp")
    temp.write_text(
        json.dumps(payload, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )
    os.replace(temp, target)
    return target
