from __future__ import annotations
from pathlib import Path
import argparse
import shutil
import sys

class PatchError(RuntimeError):
    pass

def replace_once(path: Path, old: str, new: str, label: str) -> None:
    text = path.read_text(encoding="utf-8")
    count = text.count(old)
    if count != 1:
        raise PatchError(f"{label}: expected exactly one anchor in {path}, found {count}")
    path.write_text(text.replace(old, new, 1), encoding="utf-8", newline="\n")

def ensure_absent(path: Path, marker: str) -> None:
    if marker in path.read_text(encoding="utf-8"):
        raise PatchError(f"Patch marker already present in {path}; refusing double patch")

def apply(repo: Path, overlay: Path) -> None:
    am_h = repo / "src/activities/ActivityManager.h"
    am_cpp = repo / "src/activities/ActivityManager.cpp"
    home_h = repo / "src/activities/home/HomeActivity.h"
    home_cpp = repo / "src/activities/home/HomeActivity.cpp"

    for p in (am_h, am_cpp, home_h, home_cpp):
        if not p.is_file():
            raise PatchError(f"Required upstream file missing: {p}")

    ensure_absent(am_h, "MULTIHUB")
    ensure_absent(am_cpp, "goToMultiHub()")

    replace_once(
        am_h,
        "enum class HomeMenuItem { NONE, FILE_BROWSER, RECENTS, OPDS_BROWSER, FILE_TRANSFER, SETTINGS_MENU };\n",
        "enum class HomeMenuItem { NONE, MULTIHUB, FILE_BROWSER, RECENTS, OPDS_BROWSER, FILE_TRANSFER, SETTINGS_MENU };\n",
        "ActivityManager HomeMenuItem",
    )
    replace_once(
        am_h,
        "  void goToFileBrowser(std::string path = {});\n",
        "  void goToFileBrowser(std::string path = {});\n"
        "  void goToMultiHub();\n",
        "ActivityManager goToMultiHub declaration",
    )
    replace_once(
        am_cpp,
        '#include "home/RecentBooksActivity.h"\n',
        '#include "home/RecentBooksActivity.h"\n'
        '#include "multihub/MultiHubActivity.h"\n',
        "ActivityManager MultiHub include",
    )
    replace_once(
        am_cpp,
        "void ActivityManager::goToRecentBooks() {\n",
        "void ActivityManager::goToMultiHub() {\n"
        "  replaceActivity(std::make_unique<MultiHubActivity>(renderer, mappedInput));\n"
        "}\n"
        "void ActivityManager::goToRecentBooks() {\n",
        "ActivityManager goToMultiHub implementation",
    )
    replace_once(
        am_cpp,
        '    if (activityName == "FileBrowser") {\n',
        '    if (activityName == "MultiHub") {\n'
        '      initialMenuItem = HomeMenuItem::MULTIHUB;\n'
        '    } else if (activityName == "FileBrowser") {\n',
        "ActivityManager MultiHub home return mapping",
    )

    replace_once(
        home_h,
        "    if (item == HomeMenuItem::FILE_BROWSER) return i;\n",
        "    if (item == HomeMenuItem::MULTIHUB) return i;\n"
        "    ++i;\n"
        "    if (item == HomeMenuItem::FILE_BROWSER) return i;\n",
        "Home MultiHub mapping forward",
    )
    replace_once(
        home_h,
        "    if (idx == i++) return HomeMenuItem::FILE_BROWSER;\n",
        "    if (idx == i++) return HomeMenuItem::MULTIHUB;\n"
        "    if (idx == i++) return HomeMenuItem::FILE_BROWSER;\n",
        "Home MultiHub mapping reverse",
    )
    replace_once(
        home_h,
        "  void onFileBrowserOpen();\n",
        "  void onFileBrowserOpen();\n"
        "  void onMultiHubOpen();\n",
        "Home MultiHub declaration",
    )

    replace_once(
        home_cpp,
        "  int count = 4;  // File Browser, Recents, File transfer, Settings\n",
        "  int count = 5;  // X4 MultiHub, File Browser, Recents, File transfer, Settings\n",
        "Home base menu count",
    )
    replace_once(
        home_cpp,
        "      case HomeMenuItem::FILE_BROWSER:\n"
        "        onFileBrowserOpen();\n"
        "        break;\n",
        "      case HomeMenuItem::MULTIHUB:\n"
        "        onMultiHubOpen();\n"
        "        break;\n"
        "      case HomeMenuItem::FILE_BROWSER:\n"
        "        onFileBrowserOpen();\n"
        "        break;\n",
        "Home MultiHub action",
    )
    replace_once(
        home_cpp,
        "  std::vector<const char*> menuItems = {tr(STR_BROWSE_FILES), tr(STR_MENU_RECENT_BOOKS), tr(STR_FILE_TRANSFER),\n",
        "  std::vector<const char*> menuItems = {\"X4 MultiHub\", tr(STR_BROWSE_FILES), "
        "tr(STR_MENU_RECENT_BOOKS), tr(STR_FILE_TRANSFER),\n",
        "Home MultiHub menuItems",
    )
    replace_once(
        home_cpp,
        "  std::vector<UIIcon> menuIcons = {Folder, Recent, Transfer, Settings};\n",
        "  std::vector<UIIcon> menuIcons = {Library, Folder, Recent, Transfer, Settings};\n",
        "Home MultiHub menuIcons",
    )
    replace_once(
        home_cpp,
        "    menuItems.insert(menuItems.begin() + 2, tr(STR_OPDS_BROWSER));\n"
        "    menuIcons.insert(menuIcons.begin() + 2, Library);\n",
        "    menuItems.insert(menuItems.begin() + 3, tr(STR_OPDS_BROWSER));\n"
        "    menuIcons.insert(menuIcons.begin() + 3, Library);\n",
        "Home OPDS insertion offset",
    )
    replace_once(
        home_cpp,
        "void HomeActivity::onFileBrowserOpen() { activityManager.goToFileBrowser(); }\n",
        "void HomeActivity::onFileBrowserOpen() { activityManager.goToFileBrowser(); }\n"
        "void HomeActivity::onMultiHubOpen() { activityManager.goToMultiHub(); }\n",
        "Home MultiHub implementation",
    )

    dest = repo / "src/multihub"
    if dest.exists():
        raise PatchError(f"{dest} already exists; refusing to overwrite")
    shutil.copytree(overlay / "src/multihub", dest)
    print("X4 MultiHub v1.2-dev Power Manager patch applied safely.")

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("repo")
    ap.add_argument(
        "--overlay",
        default=str(Path(__file__).resolve().parents[1] / "overlay"),
    )
    ns = ap.parse_args()
    apply(Path(ns.repo).resolve(), Path(ns.overlay).resolve())
    return 0

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except PatchError as exc:
        print("PATCH ERROR:", exc, file=sys.stderr)
        raise SystemExit(2)
