from __future__ import annotations
from pathlib import Path

REQUIRED_MARKERS = (
    b"X4 MultiHub",
    b"X4 MultiHub 1.2-dev",
    b"Wyczysc caly cache",
    b"Wyczysc cache pogody",
    b"Wyczysc cache rynkow",
    b"Ustawienia MultiHub",
    b"Gateway / Pogoda / Cache / Dashboard",
    b"Planer",
    b"Instrukcje",
    b"Czytnik",
    b"Reader",
    b"Biblioteka i pliki",
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
    b"Gateway http://IP:8788",
    b"DATA UNAVAILABLE",
    b"Odswiez notowania",
    b"Szukaj i dodaj",
    b"X4 Data Gateway",
    b"GPW / NewConnect / Crypto / FX",
    b"Dashboard",
    b"CACHED:",
    b"Dashboard CACHED",
    b"Pogoda CACHED",
    b"Markets CACHED",
    b"Show/Hide | Hold: up",
    b"Odswiez Dashboard",
    b"Uklad Dashboard",
    b"Wilgotnosc",
    b"Odswiez pogode",
    b"Open-Meteo przez Gateway",
    b"Wiadomosci",
    b"RSS / Atom / Ulubione",
    b"News Terminal",
    b"Dodaj zrodlo RSS/Atom",
    b"Ulubione wiadomosci",
    b"Odswiez kanal",
    b"URL RSS/Atom http(s)://",
    b"Power Manager",
    b"Online przy otwarciu",
    b"Wi-Fi OFF po odswiezeniu",
    b"Wylacz Wi-Fi teraz",
    b"Pogoda / Rynki / Planner",
)

def verify_required_markers(path: Path) -> list[str]:
    data = path.read_bytes()
    missing = [
        marker.decode("utf-8", errors="replace")
        for marker in REQUIRED_MARKERS
        if marker not in data
    ]
    return ["Missing required firmware markers: " + ", ".join(missing)] if missing else []
