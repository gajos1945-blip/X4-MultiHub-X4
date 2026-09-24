# X4 Data Gateway 1.5

X4 talks to one LAN gateway. The gateway talks to external providers.

Providers:
- EODHD: WAR / CC / FOREX search and quotes
- Open-Meteo: geocoding and weather
- RSS / Atom: News Terminal feeds

## Windows GUI

GitHub Actions builds:

`X4DataGateway.exe`

The GUI:
- starts/stops the LAN gateway,
- shows the LAN URL to enter on the X4,
- performs `/health` checks,
- stores the EODHD token using Windows DPAPI,
- keeps provider secrets out of firmware and Git.

Default port: `8788`.

## Command-line mode

Environment variables:
- `EODHD_API_TOKEN`
- `MULTIHUB_GATEWAY_HOST` (default `0.0.0.0`)
- `MULTIHUB_GATEWAY_PORT` (default `8788`)

Run:

```bat
python server.py
```

## Routes

- `GET /health`
- `GET /v1/search?asset=pl&q=cd`
- `GET /v1/search?asset=crypto&q=bitcoin`
- `GET /v1/search?asset=fx&q=eur`
- `GET /v1/quote?symbol=CDR.WAR`
- `GET /v1/quotes?symbols=CDR.WAR,BTC-USD.CC`
- `GET /v1/weather?city=Katowice`
- `GET /v1/news?url=<public RSS/Atom URL>&limit=15`

## News security

RSS/Atom:
- accepts only `http` / `https`,
- rejects credentials in URLs,
- rejects DNS targets resolving to non-public addresses,
- revalidates redirects,
- caps the feed body at 1 MiB,
- returns at most 20 articles.

## LAN security

The gateway is intended for a trusted local network. It binds to `0.0.0.0` so
the X4 can reach it from the same LAN. Do not port-forward it directly to the Internet.
