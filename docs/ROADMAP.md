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
- cache
- offline fallback
- retry/backoff
- SD validation
- provider error states

## v0.8-dev — UX polish
- Polish strings
- e-ink refresh optimization
- icons/layout
- navigation consistency

## v0.9-rc
- integration tests
- static BIN inspection
- feature marker checks
- hardware test checklist

## v1.0.0
Only after physical X4 validation.
