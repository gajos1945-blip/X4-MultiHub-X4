# X4 MultiHub architecture

## Principle

X4 remains a low-RAM e-ink client. Heavy network/API work is normalized by a
small LAN service named **X4 Data Gateway**.

```text
XTEINK X4
  |
  | HTTP/JSON on LAN
  v
X4 Data Gateway
  |-- EODHD
  |    |-- WAR (GPW + instruments available in provider's WAR catalogue)
  |    |-- CC (crypto)
  |    `-- FOREX
  |
  `-- Open-Meteo
       |-- geocoding
       `-- current / forecast weather
```

Provider API keys are never compiled into firmware.

## Firmware modules

- `Reader`
- `FieldManual`
- `Planner`
- `Markets`
- `Dashboard`
- `Settings`
- `Storage`
- `GatewayClient`

The first development version only installs the MultiHub shell. Unimplemented
modules are labelled as such in the UI.

## Storage target

Planned microSD root:

```text
/.x4-multihub/
  config.json
  favorites.json
  cache/
    markets/
    weather/
  manuals/
  planner/
  reader/
```

No runtime data is baked into the application BIN.

## Market symbols

Symbols are dynamic and discovered from the provider catalogue. Examples are
not hardcoded as the user's only choices.

The gateway stores canonical provider symbols such as:
- `CDR.WAR`
- `BTC-USD.CC`

If a symbol cannot be confirmed by the provider, the client must display
`DATA UNAVAILABLE` instead of inventing a value.
