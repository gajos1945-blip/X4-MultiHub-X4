# Roadmap

## v0.1-dev — foundation
- repository/build architecture
- MultiHub Home entry
- MultiHub shell screen
- provider gateway foundation
- CI safety gates
- tests

## v0.2-dev — Reader integration
- [DONE] Reader entry routes into CrossPoint file browser / reader workflow
- [DEFERRED] Dedicated return-to-MultiHub path; stock reader navigation remains unchanged
- [DONE] compile-time and marker smoke checks

## v0.3-dev — Field Manual
- [DONE] microSD manual index rooted at /Manuals
- [DONE] dynamic categories from first folder
- [DONE] search by title/category/path
- [DONE] favorites persisted across rebuild
- [DONE] interactive .checklist files
- [DONE] TXT/Markdown/EPUB/XTC routing to CrossPoint reader

## v0.4-dev — Daily Planner
- [DONE] persistent JSONL task model
- [DONE] manually selected active date
- [DONE] previous/next day navigation
- [DONE] notes
- [DONE] priority/status
- [DONE] open/all/done views
- [DONE] task deletion and date move
- [DEFERRED] automatic Today from RTC/NTP until device clock behavior is verified

## v0.5-dev — Markets
- [DONE] gateway URL setting inside Markets
- [DONE] WAR/CC/FOREX dynamic search
- [DONE] favorites on microSD
- [DONE] batch quote refresh
- [DONE] price/change/timestamp transport from gateway
- [DONE] explicit DATA UNAVAILABLE handling
- [DONE] no fixed stock/crypto whitelist in firmware

## v0.6-dev — Weather + Dashboard
- [DONE] city query through Open-Meteo geocoding on gateway
- [DONE] current conditions
- [DONE] 4-day forecast
- [DONE] Dashboard cards for Weather / Markets / Planner
- [DONE] persistent card visibility
- [DONE] custom card ordering on device

## v0.7-dev — resilience
- [DONE] Markets cache on microSD
- [DONE] Weather cache on microSD
- [DONE] cache-first startup
- [DONE] offline/network failure fallback
- [DONE] LIVE / CACHED / DATA UNAVAILABLE states
- [DONE] gateway provider retry/backoff
- [DONE] bounded/atomic cache writes
- [DEFERRED] elapsed cache age until RTC/NTP is hardware-verified

## v0.8-dev — UX polish
- [DONE] Polish-oriented main-menu wording (ASCII-safe until font validation)
- [DONE] central MultiHub Settings
- [DONE] gateway/city/planner date in one place
- [DONE] Dashboard layout entry from Settings
- [DONE] cache clearing controls
- [DONE] Wi-Fi status and About/version row
- [DONE] navigation consistency for all main modules
- [DEFERRED] full Polish diacritics until physical font rendering is verified

## v0.9-rc1 — Release Candidate
- [DONE] integration tests
- [DONE] RC source audit
- [DONE] full static ESP BIN inspection
- [DONE] internal checksum validation
- [DONE] appended SHA-256 validation
- [DONE] feature marker checks
- [DONE] application partition spare-space reporting
- [DONE] hardware smoke-test checklist
- [WAITING] physical X4 validation

## v1.0.0
Only after physical X4 validation.
