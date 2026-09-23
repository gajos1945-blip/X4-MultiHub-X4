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

    def weather_for_city(self, city: str) -> dict:
        value = asdict(self.weather.weather_for_city(city))
        return value
