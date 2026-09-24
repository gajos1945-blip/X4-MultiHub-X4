from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / ".builder/overlay/src/multihub"

def test_planner_sources_exist():
    for name in (
        "PlannerStore.h",
        "PlannerStore.cpp",
        "DailyPlannerActivity.h",
        "DailyPlannerActivity.cpp",
        "PlannerTaskActionsActivity.h",
        "PlannerTaskActionsActivity.cpp",
    ):
        assert (OVERLAY / name).is_file()

def test_multihub_opens_real_planner():
    source = (OVERLAY / "MultiHubActivity.cpp").read_text(encoding="utf-8")
    assert '#include "DailyPlannerActivity.h"' in source
    assert "std::make_unique<DailyPlannerActivity>" in source
    assert "Daily Planner - NOT IMPLEMENTED" not in source

def test_planner_storage_and_date_guards_present():
    header = (OVERLAY / "PlannerStore.h").read_text(encoding="utf-8")
    source = (OVERLAY / "PlannerStore.cpp").read_text(encoding="utf-8")
    assert 'TASKS_PATH = "/.x4-multihub/planner/tasks.jsonl"' in header
    assert 'ACTIVE_DATE_PATH = "/.x4-multihub/planner/active_date.txt"' in header
    assert "MAX_TASKS = 400" in header
    assert "validDate" in source
    assert "shiftDate" in source
    assert "y < 2000 || y > 2099" in source

def test_planner_features_present():
    main = (OVERLAY / "DailyPlannerActivity.cpp").read_text(encoding="utf-8")
    actions = (OVERLAY / "PlannerTaskActionsActivity.cpp").read_text(encoding="utf-8")
    for token in (
        "Dodaj zadanie",
        "Poprzedni dzien",
        "Nastepny dzien",
        "Wszystkie",
        "Otwarte",
        "Wykonane",
        "Data YYYY-MM-DD",
    ):
        assert token in main or token in actions
    for token in ("Priorytet", "Notatka", "Usun zadanie"):
        assert token in actions

def test_planner_auto_today_is_opt_in_and_hardware_unverified():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    source = (OVERLAY / "DailyPlannerActivity.cpp").read_text(encoding="utf-8")
    assert data["version"] == "1.7-dev"
    assert any("Planner Today" in x for x in data["implemented"])
    assert "TimeService::applyTodayToPlanner" in source
    assert data["physical_device_verified"] is False

def test_release_guard_requires_planner_markers():
    guard = (ROOT / ".builder/tools/release_guard.py").read_text(encoding="utf-8")
    for marker in (
        "Zadania / priorytety / notatki",
        "Dodaj zadanie",
        "Poprzedni dzien",
        "Nastepny dzien",
        "Data YYYY-MM-DD",
        "Priorytet",
    ):
        assert marker in guard
