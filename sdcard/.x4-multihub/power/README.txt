X4 MultiHub Power Manager v1.2-dev

Runtime config:
  /.x4-multihub/power/config.json

Defaults:
  auto_refresh_on_open = false
  radio_off_after_refresh = false

Meaning:
- "Online przy otwarciu: RECZNIE"
  News Terminal does not automatically perform a network request simply by
  entering a feed. Use "Odswiez kanal" explicitly.

- "Wi-Fi OFF po odswiezeniu"
  This is opt-in and OFF by default.
  When enabled, MultiHub switches Wi-Fi off only after the complete online
  action/batch has finished.

- "Wylacz Wi-Fi teraz"
  Explicitly calls Arduino-ESP32 WiFi.disconnect(true, false):
  radio off, saved AP config is NOT intentionally erased.

Hardware deep sleep, wake timers and low-battery policy are NOT IMPLEMENTED in
this milestone because they require physical X4 validation.
