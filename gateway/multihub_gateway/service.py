from __future__ import annotations
from dataclasses import asdict

from .providers.eodhd import EodhdProvider
from .providers.open_meteo import OpenMeteoProvider

class MultiHubService:
    def __init__(self, eodhd: EodhdProvider, weather: OpenMeteoProvider):
        self.eodhd = eodhd
        self.weather = weather

    def search(self, asset: str, query: str) -> dict:
        mapping = {
            "pl": "WAR",
            "stocks": "WAR",
            "crypto": "CC",
            "fx": "FOREX",
        }
        exchange = mapping.get(asset.strip().lower())
        if not exchange:
            raise ValueError("asset must be pl, stocks, crypto or fx")
        items = self.eodhd.search(exchange, query)
        return {
            "asset": asset,
            "query": query,
            "exchange": exchange,
            "results": [asdict(x) for x in items],
        }

    def quote(self, symbol: str) -> dict:
        return asdict(self.eodhd.quote(symbol))

    def quotes(self, symbols: list[str]) -> dict:
        clean = [x.strip().upper() for x in symbols if x.strip()]
        if not clean:
            return {"quotes": []}
        if len(clean) > 20:
            raise ValueError("maximum 20 symbols per batch")

        found = {q.symbol: q for q in self.eodhd.quotes(clean)}
        rows = []
        for symbol in clean:
            quote = found.get(symbol)
            if quote is None:
                rows.append({
                    "symbol": symbol,
                    "available": False,
                    "price": None,
                    "change_p": None,
                    "timestamp": None,
                    "provider": "EODHD",
                })
            else:
                rows.append({
                    "symbol": quote.symbol,
                    "available": quote.price is not None,
                    "price": quote.price,
                    "change_p": quote.change_p,
                    "timestamp": quote.timestamp,
                    "provider": quote.provider,
                })
        return {"quotes": rows}

    def weather_for_city(self, city: str) -> dict:
        value = asdict(self.weather.weather_for_city(city))
        return value
