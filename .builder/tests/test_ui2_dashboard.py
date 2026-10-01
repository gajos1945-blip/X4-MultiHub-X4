from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / ".builder" / "overlay" / "src" / "multihub"


def test_ui2_dashboard_is_graphical_freeink_screen():
    h = (OVERLAY / "MultiHubActivity.h").read_text(encoding="utf-8")
    cpp = (OVERLAY / "MultiHubActivity.cpp").read_text(encoding="utf-8")
    assert "private UiAppHost" in h
    assert "MAIN_TILES = 8" in h
    assert "QUICK_TILES = 3" in h
    assert "Centrum czytania, organizacji i informacji" in cpp
    assert "drawMainTile" in cpp
    assert "drawQuickTile" in cpp
    assert "screen.frame().hit" in cpp


def test_ui2_preserves_all_navigation_targets():
    cpp = (OVERLAY / "MultiHubActivity.cpp").read_text(encoding="utf-8")
    for marker in (
        "ReaderDashboardActivity",
        "activityManager.goToFileBrowser();",
        "FieldManualActivity",
        "DailyPlannerActivity",
        "NewsActivity",
        "WeatherActivity",
        "MarketsActivity",
        "MultiHubSettingsActivity",
        "DashboardActivity",
        "DiagnosticsActivity",
    ):
        assert marker in cpp


def test_ui2_x4c_button_navigation_present():
    cpp = (OVERLAY / "MultiHubActivity.cpp").read_text(encoding="utf-8")
    for marker in (
        "Button::ScreenLeft",
        "Button::ScreenRight",
        "Button::ScreenUp",
        "Button::ScreenDown",
        "Button::Confirm",
        "Button::Back",
        "Button::NavPrevious",
        "Button::NavNext",
    ):
        assert marker in cpp


def test_gateway_quick_action_selects_gateway_settings_row():
    h = (OVERLAY / "MultiHubSettingsActivity.h").read_text(encoding="utf-8")
    cpp = (OVERLAY / "MultiHubActivity.cpp").read_text(encoding="utf-8")
    assert "int initialSelection = 0" in h
    assert "MultiHubSettingsActivity>(renderer, mappedInput, 1)" in cpp
