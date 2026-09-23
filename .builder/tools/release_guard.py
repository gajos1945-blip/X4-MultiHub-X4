from __future__ import annotations
from pathlib import Path

REQUIRED_MARKERS = (
    b"X4 MultiHub",
    b"Reader",
    b"Field Manual",
    b"Daily Planner",
    b"Markets & Weather",
    b"Dashboard",
    b"NOT IMPLEMENTED v0.1",
)

def verify_required_markers(path: Path) -> list[str]:
    data = path.read_bytes()
    missing = [
        marker.decode("utf-8", errors="replace")
        for marker in REQUIRED_MARKERS
        if marker not in data
    ]
    return ["Missing required firmware markers: " + ", ".join(missing)] if missing else []
