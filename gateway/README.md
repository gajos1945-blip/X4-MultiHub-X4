# X4 Data Gateway v0.1

The X4 talks to one LAN service. The service talks to external providers.

Providers:
- EODHD: WAR / CC / quote
- Open-Meteo: geocoding and weather

## Secrets

Never put `EODHD_API_TOKEN` into firmware or commit it to GitHub.

Windows example:

```bat
set EODHD_API_TOKEN=YOUR_TOKEN
set MULTIHUB_GATEWAY_HOST=0.0.0.0
set MULTIHUB_GATEWAY_PORT=8788
START_GATEWAY_WINDOWS.cmd
```

## Routes

- `GET /health`
- `GET /v1/search?asset=pl&q=cd`
- `GET /v1/search?asset=crypto&q=bitcoin`
- `GET /v1/quote?symbol=CDR.WAR`
- `GET /v1/weather?city=Katowice`

The gateway returns normalized JSON. The X4 firmware never needs to know the
provider's private API token.
