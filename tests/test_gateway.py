from __future__ import annotations
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "gateway"))

from multihub_gateway.providers.eodhd import EodhdProvider
from multihub_gateway.providers.open_meteo import OpenMeteoProvider

class FakeHttp:
    def __init__(self, responses):
        self.responses = list(responses)
        self.urls = []

    def get_json(self, url, timeout=30.0):
        self.urls.append(url)
        return self.responses.pop(0)

def test_eodhd_search_is_dynamic_not_fixed():
    fake = FakeHttp([[
        {"Code": "CDR", "Name": "CD PROJEKT SA", "Currency": "PLN",
         "Type": "Common Stock", "Isin": "PLOPTTC00011"},
        {"Code": "YOS", "Name": "Yoshi Innovation SA", "Currency": "PLN",
         "Type": "Common Stock", "Isin": "PLTEST000001"},
    ]])
    p = EodhdProvider("secret", fake)
    items = p.search("WAR", "yoshi")
    assert len(items) == 1
    assert items[0].symbol == "YOS.WAR"
    assert "exchange-symbol-list/WAR" in fake.urls[0]

def test_crypto_catalog_uses_cc():
    fake = FakeHttp([[
        {"Code": "BTC-USD", "Name": "Bitcoin USD", "Currency": "USD",
         "Type": "Currency", "Isin": None}
    ]])
    p = EodhdProvider("secret", fake)
    items = p.search("CC", "bitcoin")
    assert items[0].symbol == "BTC-USD.CC"
    assert "exchange-symbol-list/CC" in fake.urls[0]

def test_quote_normalization():
    fake = FakeHttp([{
        "code": "CDR.WAR",
        "close": 321.5,
        "open": 318.0,
        "high": 325.0,
        "low": 315.0,
        "previousClose": 319.0,
        "change": 2.5,
        "change_p": 0.78,
        "timestamp": 1234567890,
    }])
    p = EodhdProvider("secret", fake)
    q = p.quote("CDR.WAR")
    assert q.price == 321.5
    assert q.timestamp == 1234567890
    assert "real-time/CDR.WAR" in fake.urls[0]

def test_weather_geocode_and_current():
    fake = FakeHttp([
        {"results": [{
            "name": "Katowice", "country": "Polska",
            "latitude": 50.25, "longitude": 19.02,
            "timezone": "Europe/Warsaw"
        }]},
        {"current": {
            "temperature_2m": 14.2,
            "apparent_temperature": 13.7,
            "relative_humidity_2m": 71,
            "precipitation": 0.0,
            "weather_code": 2,
            "wind_speed_10m": 8.4,
            "time": "2026-09-23T06:30",
        }},
    ])
    p = OpenMeteoProvider(fake)
    w = p.weather_for_city("Katowice")
    assert w.place.name == "Katowice"
    assert w.temperature_c == 14.2
    assert w.humidity_percent == 71.0


def test_batch_quotes_use_eodhd_s_parameter():
    fake = FakeHttp([[
        {
            "code": "CDR.WAR", "close": 300.0, "change_p": 1.0,
            "timestamp": 100, "open": 295.0, "high": 301.0, "low": 294.0,
            "previousClose": 297.0,
        },
        {
            "code": "BTC-USD.CC", "close": 100000.0, "change_p": -0.5,
            "timestamp": 101, "open": 101000.0, "high": 102000.0, "low": 99000.0,
            "previousClose": 100500.0,
        },
    ]])
    p = EodhdProvider("secret", fake)
    rows = p.quotes(["CDR.WAR", "BTC-USD.CC"])
    assert [x.symbol for x in rows] == ["CDR.WAR", "BTC-USD.CC"]
    assert "real-time/CDR.WAR" in fake.urls[0]
    assert "s=BTC-USD.CC" in fake.urls[0]

def test_batch_quotes_reject_too_many_symbols():
    fake = FakeHttp([])
    p = EodhdProvider("secret", fake)
    try:
        p.quotes([f"X{i}.WAR" for i in range(21)])
        assert False, "should reject >20 symbols"
    except ValueError:
        pass


def test_weather_forecast_is_transported():
    fake = FakeHttp([
        {"results": [{
            "name": "Katowice", "country": "Polska",
            "latitude": 50.25, "longitude": 19.02,
            "timezone": "Europe/Warsaw"
        }]},
        {
            "current": {
                "temperature_2m": 14.2,
                "apparent_temperature": 13.7,
                "relative_humidity_2m": 71,
                "precipitation": 0.0,
                "weather_code": 2,
                "wind_speed_10m": 8.4,
                "time": "2026-09-23T11:30",
            },
            "daily": {
                "time": ["2026-09-23", "2026-09-24", "2026-09-25"],
                "weather_code": [2, 3, 61],
                "temperature_2m_min": [9.0, 8.0, 7.0],
                "temperature_2m_max": [16.0, 15.0, 13.0],
                "precipitation_probability_max": [10, 20, 70],
            },
        },
    ])
    p = OpenMeteoProvider(fake)
    w = p.weather_for_city("Katowice")
    assert len(w.forecast) == 3
    assert w.forecast[0].max_c == 16.0
    assert w.forecast[2].weather_code == 61
    assert "daily=" in fake.urls[1]
    assert "forecast_days=4" in fake.urls[1]
