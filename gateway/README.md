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
- `GET /v1/quotes?symbols=CDR.WAR,BTC-USD.CC`
- `GET /v1/weather?city=Katowice`

The gateway returns normalized JSON. The X4 firmware never needs to know the
provider's private API token.


## News Terminal v1.1

Endpoint:
- `GET /v1/news?url=<public RSS/Atom URL>&limit=15`

Security:
- only `http` / `https`,
- credentials in URLs are rejected,
- DNS targets resolving to loopback/private/link-local/reserved addresses are rejected,
- redirects are revalidated,
- feed body is capped at 1 MiB,
- at most 20 articles are returned.

The gateway parses RSS/Atom and sends compact JSON to the X4.
No RSS credentials/API secrets are stored in firmware.
