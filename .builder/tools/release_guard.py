from __future__ import annotations
from pathlib import Path

REQUIRED_MARKERS = (
    b"X4 MultiHub",
    b"Reader",
    b"Otworz biblioteke / pliki",
    b"Field Manual",
    b"Manuale offline / checklisty",
    b"Checklist",
    b"Przebuduj indeks",
    b"/Manuals",
    b"Daily Planner",
    b"Priorytet",
    b"Data YYYY-MM-DD",
    b"Nastepny dzien",
    b"Poprzedni dzien",
    b"Dodaj zadanie",
    b"Zadania / priorytety / notatki",
    b"Markets & Weather",
    b"Dashboard",
    b"NOT IMPLEMENTED v0.4",
)

def verify_required_markers(path: Path) -> list[str]:
    data = path.read_bytes()
    missing = [
        marker.decode("utf-8", errors="replace")
        for marker in REQUIRED_MARKERS
        if marker not in data
    ]
    return ["Missing required firmware markers: " + ", ".join(missing)] if missing else []
