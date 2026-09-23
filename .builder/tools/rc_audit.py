from __future__ import annotations

from pathlib import Path

REQUIRED_MODULES = (
    "MultiHubActivity.cpp",
    "MultiHubSettingsActivity.cpp",
    "FieldManualActivity.cpp",
    "DailyPlannerActivity.cpp",
    "MarketsActivity.cpp",
    "WeatherActivity.cpp",
    "DashboardActivity.cpp",
    "DataCache.cpp",
)

FORBIDDEN_OVERLAY_TEXT = (
    "erase_flash",
    "EODHD_API_TOKEN",
    "eodhd.com",
    "open-meteo.com",
    "NOT IMPLEMENTED v0.",
)


def audit_overlay(root: Path) -> list[str]:
    errors: list[str] = []
    overlay = root / ".builder" / "overlay" / "src" / "multihub"

    for name in REQUIRED_MODULES:
        if not (overlay / name).is_file():
            errors.append(f"missing RC module: {name}")

    chunks: list[str] = []
    for path in overlay.rglob("*"):
        if path.is_file() and path.suffix in {".cpp", ".h"}:
            chunks.append(path.read_text(encoding="utf-8", errors="replace"))
    joined = "\n".join(chunks)

    for token in FORBIDDEN_OVERLAY_TEXT:
        if token in joined:
            errors.append(f"forbidden RC overlay token: {token}")

    return errors


def audit_repository(root: Path) -> list[str]:
    errors = audit_overlay(root)

    build_ci = (root / ".builder" / "build_ci.py").read_text(
        encoding="utf-8", errors="replace"
    )
    workflow = (root / ".github" / "workflows" / "build-x4-bin.yml").read_text(
        encoding="utf-8", errors="replace"
    )

    if "erase_flash" in build_ci or "erase_flash" in workflow:
        errors.append("automatic erase_flash must not exist in CI/build")

    if '"merged_full_flash": False' not in build_ci:
        errors.append("RC manifest must explicitly mark merged_full_flash false")

    return errors
