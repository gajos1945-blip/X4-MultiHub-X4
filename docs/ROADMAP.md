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

## v1.0.0-rc1 — final software candidate
- [DONE] software feature set complete
- [DONE] final RC packaging
- [DONE] Custom.bin byte-identical alias
- [DONE] static integrity and partition gates
- [WAITING] physical X4 validation

## v1.0.0 stable
Only after physical X4 validation.


## v1.1-dev — News Terminal
- [DONE] user-managed RSS/Atom sources
- [DONE] News Terminal main screen
- [DONE] feed headlines/source/publication date
- [DONE] article summary/content from feed opened in CrossPoint reader
- [DONE] persistent News favorites
- [DONE] gateway RSS/Atom parser
- [DONE] SSRF protection and redirect revalidation
- [DONE] 1 MiB feed limit / 20 article cap
- [DEFERRED] full web-page readability extraction
- [DEFERRED] news offline cache


## v1.2-dev — Power Manager
- [DONE] persistent power policy
- [DONE] News manual refresh by default
- [DONE] opt-in Wi-Fi OFF after a completed online operation
- [DONE] explicit Wi-Fi OFF action
- [DONE] no intentional AP credential erase
- [DEFERRED] deep-sleep/wake-source modifications until physical X4 validation
- [DEFERRED] low-battery threshold until battery API is physically verified


## v1.3-dev — Time / NTP / Today
- [DONE] manual SNTP sync on an already-connected Wi-Fi session
- [DONE] explicit timezone presets / POSIX TZ rules
- [DONE] opt-in Planner Auto Today
- [DONE] cache write timestamps when clock is valid
- [DONE] cache age labels in Markets / Weather / Dashboard
- [SAFE DEFAULT] Auto Today off
- [DEFERRED] X4 RTC/deep-sleep accuracy claims until physical validation
- [DEFERRED] automatic Wi-Fi connection solely for NTP


## v1.4-dev — Reader Dashboard
- [DONE] current/last open book from CrossPoint APP_STATE
- [DONE] title/author from CrossPoint RecentBooksStore
- [DONE] EPUB chapter page progress from CrossPoint progress.bin
- [DONE] whole-book percentage via cached EPUB metadata
- [DONE] Continue Reading back into stock CrossPoint reader
- [DONE] Reader card visibility/order in Dashboard
- [SAFE] MultiHub never writes CrossPoint progress.bin
- [DEFERRED] equivalent whole-book percentage for formats without the same cached EPUB metadata API


## v1.6-dev — Gateway Access Control
- [DONE] optional shared token for all `/v1/*` gateway APIs
- [DONE] `X-X4-Token` support in Markets / Weather / News
- [DONE] token stored on X4 microSD, not compiled into firmware
- [DONE] constant-time comparison in gateway
- [DONE] token generation/copy in Windows Gateway
- [DONE] DPAPI protection for both Windows-side secrets
- [LIMITATION] HTTP LAN transport is not encrypted
- [DEFERRED] TLS between X4 and gateway


## v1.7-dev — Diagnostics
- [DONE] Wi-Fi / RSSI status
- [DONE] Gateway `/health` test
- [DONE] authenticated local `/v1/ping` token-match test
- [DONE] microSD write/read/remove self-test
- [DONE] local-time status
- [DONE] diagnostics report export with token value redacted
- [DONE] Windows Gateway "Test X4 auth"
- [SAFE] `/v1/ping` never contacts external providers
- [DEFERRED] remote diagnostics upload
