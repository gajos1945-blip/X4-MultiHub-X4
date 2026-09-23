from __future__ import annotations
import json
import time
from typing import Any
import urllib.error
import urllib.request

class HttpError(RuntimeError):
    pass

class JsonHttpClient:
    RETRYABLE_HTTP = {429, 500, 502, 503, 504}

    def get_json(
        self,
        url: str,
        timeout: float = 30.0,
        attempts: int = 3,
    ) -> Any:
        attempts = max(1, min(int(attempts), 4))
        delays = (0.0, 0.35, 1.0, 2.0)
        last_error: Exception | None = None

        for attempt in range(attempts):
            if attempt:
                time.sleep(delays[min(attempt, len(delays) - 1)])

            req = urllib.request.Request(
                url,
                method="GET",
                headers={"User-Agent": "X4-MultiHub-Gateway/0.7"},
            )
            try:
                with urllib.request.urlopen(req, timeout=timeout) as response:
                    raw = response.read()
            except urllib.error.HTTPError as exc:
                detail = exc.read().decode("utf-8", errors="replace")
                error = HttpError(f"HTTP {exc.code}: {detail[:300]}")
                if exc.code not in self.RETRYABLE_HTTP:
                    raise error from exc
                last_error = error
                continue
            except OSError as exc:
                last_error = HttpError(f"Connection error: {exc}")
                continue

            try:
                return json.loads(raw.decode("utf-8"))
            except Exception as exc:
                raise HttpError("Invalid JSON response") from exc

        if isinstance(last_error, HttpError):
            raise last_error
        raise HttpError("Provider request failed after retries")
