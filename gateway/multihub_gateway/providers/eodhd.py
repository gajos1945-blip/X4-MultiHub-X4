from __future__ import annotations
from dataclasses import dataclass
from typing import Any
import urllib.parse

from ..http_client import JsonHttpClient

BASE = "https://eodhd.com/api"

class ProviderConfigError(RuntimeError):
    pass

@dataclass(frozen=True)
class Instrument:
    symbol: str
    code: str
    name: str
    exchange: str
    currency: str
    instrument_type: str
    isin: str | None

@dataclass(frozen=True)
class Quote:
    symbol: str
    price: float | None
    open: float | None
    high: float | None
    low: float | None
    previous_close: float | None
    change: float | None
    change_p: float | None
    timestamp: int | None
    provider: str = "EODHD"

class EodhdProvider:
    def __init__(self, token: str, http: JsonHttpClient | None = None):
        self.token = token.strip()
        self.http = http or JsonHttpClient()

    def _require_token(self) -> None:
        if not self.token:
            raise ProviderConfigError("EODHD_API_TOKEN is not configured")

    def _url(self, path: str, **params: str) -> str:
        self._require_token()
        query = {"api_token": self.token, "fmt": "json", **params}
        return f"{BASE}/{path}?{urllib.parse.urlencode(query)}"

    def exchange_symbols(self, exchange: str) -> list[Instrument]:
        exchange = exchange.strip().upper()
        if exchange not in {"WAR", "CC", "FOREX"}:
            raise ValueError("unsupported exchange")
        data = self.http.get_json(self._url(f"exchange-symbol-list/{exchange}"))
        if not isinstance(data, list):
            raise RuntimeError("EODHD catalogue response is not a list")
        result: list[Instrument] = []
        for row in data:
            if not isinstance(row, dict):
                continue
            code = str(row.get("Code", "")).strip()
            if not code:
                continue
            result.append(
                Instrument(
                    symbol=f"{code}.{exchange}",
                    code=code,
                    name=str(row.get("Name", "")).strip(),
                    exchange=exchange,
                    currency=str(row.get("Currency", "")).strip(),
                    instrument_type=str(row.get("Type", "")).strip(),
                    isin=(str(row.get("Isin")).strip() if row.get("Isin") else None),
                )
            )
        return result

    def search(self, exchange: str, query: str, limit: int = 30) -> list[Instrument]:
        q = query.strip().casefold()
        if not q:
            return []
        items = self.exchange_symbols(exchange)
        matches = [
            item for item in items
            if q in item.code.casefold()
            or q in item.name.casefold()
            or (item.isin and q in item.isin.casefold())
        ]
        matches.sort(key=lambda x: (
            0 if x.code.casefold().startswith(q) else 1,
            x.name.casefold(),
            x.code.casefold(),
        ))
        return matches[:max(1, min(limit, 100))]

    def quote(self, symbol: str) -> Quote:
        symbol = symbol.strip().upper()
        if not symbol or "." not in symbol:
            raise ValueError("canonical provider symbol required")
        data = self.http.get_json(self._url(f"real-time/{urllib.parse.quote(symbol, safe='.-')}"))
        if not isinstance(data, dict):
            raise RuntimeError("EODHD quote response is not an object")

        def number(name: str) -> float | None:
            value = data.get(name)
            if value is None or value == "":
                return None
            try:
                return float(value)
            except (TypeError, ValueError):
                return None

        ts = data.get("timestamp")
        try:
            timestamp = int(ts) if ts is not None else None
        except (TypeError, ValueError):
            timestamp = None

        return Quote(
            symbol=str(data.get("code") or symbol),
            price=number("close"),
            open=number("open"),
            high=number("high"),
            low=number("low"),
            previous_close=number("previousClose"),
            change=number("change"),
            change_p=number("change_p"),
            timestamp=timestamp,
        )
