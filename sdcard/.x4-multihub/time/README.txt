X4 MultiHub Time / NTP v1.3-dev

Runtime config:
  /.x4-multihub/time/config.json

Safe defaults:
  timezone_preset = 0 (UTC)
  auto_today = false

Timezone presets:
  0 UTC
  1 Polska
  2 UK
  3 US Eastern

NTP:
- synchronization is manual from Ustawienia -> Czas / NTP,
- it only runs when Wi-Fi is already connected,
- if CrossPoint/ESP-IDF SNTP is already active, MultiHub requests a restart
  of that service rather than replacing it,
- otherwise it starts ESP-IDF SNTP with pool.ntp.org and time.google.com,
- no saved Wi-Fi credentials are changed.

Planner:
- Auto Today is OFF by default,
- when enabled, Planner uses the locally converted system date only if the
  system clock is valid,
- if time is UNKNOWN, the existing manually selected planner date is kept.

Cache:
- new Market/Weather cache writes include cached_at_epoch when system time is
  valid,
- older cache files remain readable and show simply CACHED.
