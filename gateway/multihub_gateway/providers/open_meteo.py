from __future__ import annotations
from dataclasses import dataclass, field
import urllib.parse

from ..http_client import JsonHttpClient

@dataclass(frozen=True)
class Place:
    name: str
    country: str
    latitude: float
    longitude: float
    timezone: str

@dataclass(frozen=True)
class ForecastDay:
    date: str
    weather_code: int | None
    min_c: float | None
    max_c: float | None
    precipitation_probability_max: float | None

@dataclass(frozen=True)
class CurrentWeather:
    place: Place
    temperature_c: float | None
    apparent_temperature_c: float | None
    humidity_percent: float | None
    precipitation_mm: float | None
    wind_kmh: float | None
    weather_code: int | None
    observed_at: str | None
    provider: str = "Open-Meteo"
    forecast: list[ForecastDay] = field(default_factory=list)

class OpenMeteoProvider:
    def __init__(self, http: JsonHttpClient | None = None):
        self.http = http or JsonHttpClient()

    def geocode(self, city: str, count: int = 5) -> list[Place]:
        city = city.strip()
        if not city:
            return []
        query = urllib.parse.urlencode({
            "name": city,
            "count": max(1, min(count, 10)),
            "language": "pl",
            "format": "json",
        })
        data = self.http.get_json(
            f"https://geocoding-api.open-meteo.com/v1/search?{query}"
        )
        rows = data.get("results", []) if isinstance(data, dict) else []
        result: list[Place] = []
        for row in rows:
            try:
                result.append(
                    Place(
                        name=str(row["name"]),
                        country=str(row.get("country", "")),
                        latitude=float(row["latitude"]),
                        longitude=float(row["longitude"]),
                        timezone=str(row.get("timezone", "auto")),
                    )
                )
            except (KeyError, TypeError, ValueError):
                continue
        return result

    def current(self, place: Place) -> CurrentWeather:
        query = urllib.parse.urlencode({
            "latitude": place.latitude,
            "longitude": place.longitude,
            "current": ",".join([
                "temperature_2m",
                "apparent_temperature",
                "relative_humidity_2m",
                "precipitation",
                "weather_code",
                "wind_speed_10m",
            ]),
            "daily": ",".join([
                "weather_code",
                "temperature_2m_min",
                "temperature_2m_max",
                "precipitation_probability_max",
            ]),
            "forecast_days": 4,
            "timezone": "auto",
        })
        data = self.http.get_json(f"https://api.open-meteo.com/v1/forecast?{query}")
        current = data.get("current", {}) if isinstance(data, dict) else {}

        def number(name: str) -> float | None:
            value = current.get(name)
            if value is None:
                return None
            try:
                return float(value)
            except (TypeError, ValueError):
                return None

        code = current.get("weather_code")
        try:
            weather_code = int(code) if code is not None else None
        except (TypeError, ValueError):
            weather_code = None

        daily = data.get("daily", {}) if isinstance(data, dict) else {}
        dates = daily.get("time", []) if isinstance(daily, dict) else []
        codes = daily.get("weather_code", []) if isinstance(daily, dict) else []
        mins = daily.get("temperature_2m_min", []) if isinstance(daily, dict) else []
        maxs = daily.get("temperature_2m_max", []) if isinstance(daily, dict) else []
        pops = daily.get("precipitation_probability_max", []) if isinstance(daily, dict) else []

        forecast: list[ForecastDay] = []
        count = min(len(dates), 4)
        for i in range(count):
            def at(values, index):
                try:
                    return values[index]
                except (IndexError, TypeError):
                    return None

            def float_or_none(value):
                try:
                    return float(value) if value is not None else None
                except (TypeError, ValueError):
                    return None

            raw_code = at(codes, i)
            try:
                day_code = int(raw_code) if raw_code is not None else None
            except (TypeError, ValueError):
                day_code = None

            forecast.append(
                ForecastDay(
                    date=str(at(dates, i) or ""),
                    weather_code=day_code,
                    min_c=float_or_none(at(mins, i)),
                    max_c=float_or_none(at(maxs, i)),
                    precipitation_probability_max=float_or_none(at(pops, i)),
                )
            )

        return CurrentWeather(
            place=place,
            temperature_c=number("temperature_2m"),
            apparent_temperature_c=number("apparent_temperature"),
            humidity_percent=number("relative_humidity_2m"),
            precipitation_mm=number("precipitation"),
            wind_kmh=number("wind_speed_10m"),
            weather_code=weather_code,
            observed_at=str(current.get("time")) if current.get("time") else None,
            forecast=forecast,
        )

    def weather_for_city(self, city: str) -> CurrentWeather:
        places = self.geocode(city, count=1)
        if not places:
            raise RuntimeError("location not found")
        return self.current(places[0])
