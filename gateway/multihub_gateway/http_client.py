from __future__ import annotations
import json
from typing import Any
import urllib.error
import urllib.request

class HttpError(RuntimeError):
    pass

class JsonHttpClient:
    def get_json(self, url: str, timeout: float = 30.0) -> Any:
        req = urllib.request.Request(
            url,
            method="GET",
            headers={"User-Agent": "X4-MultiHub-Gateway/0.1"},
        )
        try:
            with urllib.request.urlopen(req, timeout=timeout) as response:
                raw = response.read()
        except urllib.error.HTTPError as exc:
            detail = exc.read().decode("utf-8", errors="replace")
            raise HttpError(f"HTTP {exc.code}: {detail[:300]}") from exc
        except OSError as exc:
            raise HttpError(f"Connection error: {exc}") from exc
        try:
            return json.loads(raw.decode("utf-8"))
        except Exception as exc:
            raise HttpError("Invalid JSON response") from exc
