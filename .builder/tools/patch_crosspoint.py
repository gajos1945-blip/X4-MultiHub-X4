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
        raise PatchError(
            f"{label}: expected exactly one anchor in {path}, found {count}"
        )
    path.write_text(text.replace(old, new, 1), encoding="utf-8", newline="\n")


def ensure_absent(path: Path, marker: str) -> None:
    if marker in path.read_text(encoding="utf-8"):
        raise PatchError(f"Patch marker already present in {path}; refusing double patch")


def patch_version_label(platformio: Path) -> None:
    text = platformio.read_text(encoding="utf-8")
    header = "[env:x4c-gh_release]"
    start = text.find(header)
    if start < 0:
        raise PatchError("x4c-gh_release environment not found in platformio.ini")
    end = text.find("\n[", start + len(header))
    if end < 0:
        end = len(text)
    block = text[start:end]

    old = r'-DCROSSPOINT_VERSION=\"${crosspoint.version}\"'
    new = r'-DCROSSPOINT_VERSION=\"X4-MultiHub-1.7-hwtest\"'
    if old not in block:
        raise PatchError("CrossPoint version anchor missing in x4c-gh_release")
    block = block.replace(old, new, 1)
    platformio.write_text(text[:start] + block + text[end:], encoding="utf-8", newline="\n")


def apply(repo: Path, overlay: Path) -> None:
    am_h = repo / "src/activities/ActivityManager.h"
    am_cpp = repo / "src/activities/ActivityManager.cpp"
    home_h = repo / "src/activities/home/HomeActivity.h"
    home_cpp = repo / "src/activities/home/HomeActivity.cpp"
    grid_h = repo / "src/components/CoverGridHomeUi.h"
    grid_cpp = repo / "src/components/CoverGridHomeUi.cpp"
    platformio = repo / "platformio.ini"

    for p in (am_h, am_cpp, home_h, home_cpp, grid_h, grid_cpp, platformio):
        if not p.is_file():
            raise PatchError(f"Required CrossPoint 1.6.5 file missing: {p}")

    ensure_absent(am_h, "MULTIHUB")
    ensure_absent(am_cpp, "goToMultiHub()")

    # ActivityManager - add MultiHub as a first-class Home destination.
    replace_once(
        am_h,
        "enum class HomeMenuItem { NONE, FILE_BROWSER, LIBRARY, OPDS_BROWSER, FILE_TRANSFER, SETTINGS_MENU };\n",
        "enum class HomeMenuItem { NONE, MULTIHUB, FILE_BROWSER, LIBRARY, OPDS_BROWSER, FILE_TRANSFER, SETTINGS_MENU };\n",
        "ActivityManager HomeMenuItem",
    )
    replace_once(
        am_h,
        "  void goToSettings();\n  void goToFileBrowser(std::string path = {});\n",
        "  void goToSettings();\n  void goToMultiHub();\n  void goToFileBrowser(std::string path = {});\n",
        "ActivityManager goToMultiHub declaration",
    )
    replace_once(
        am_cpp,
        '#include "home/HomeActivity.h"\n',
        '#include "home/HomeActivity.h"\n#include "multihub/MultiHubActivity.h"\n',
        "ActivityManager MultiHub include",
    )
    replace_once(
        am_cpp,
        "void ActivityManager::goToFileBrowser(std::string path) {\n",
        "void ActivityManager::goToMultiHub() {\n"
        "  replaceActivity(std::make_unique<MultiHubActivity>(renderer, mappedInput));\n"
        "}\n\n"
        "void ActivityManager::goToFileBrowser(std::string path) {\n",
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

    # HomeActivity - classic/list theme.
    replace_once(
        home_h,
        "    int i = 0;\n    if (item == HomeMenuItem::FILE_BROWSER) return i;\n",
        "    int i = 0;\n"
        "    if (item == HomeMenuItem::MULTIHUB) return i;\n"
        "    ++i;\n"
        "    if (item == HomeMenuItem::FILE_BROWSER) return i;\n",
        "Home MultiHub mapping forward",
    )
    replace_once(
        home_h,
        "    int i = 0;\n    if (idx == i++) return HomeMenuItem::FILE_BROWSER;\n",
        "    int i = 0;\n"
        "    if (idx == i++) return HomeMenuItem::MULTIHUB;\n"
        "    if (idx == i++) return HomeMenuItem::FILE_BROWSER;\n",
        "Home MultiHub mapping reverse",
    )
    replace_once(
        home_h,
        "  void onFileBrowserOpen();\n",
        "  void onMultiHubOpen();\n  void onFileBrowserOpen();\n",
        "Home MultiHub declaration",
    )
    replace_once(
        home_cpp,
        "  int count = 4;  // File Browser, Library, File transfer, Settings\n",
        "  int count = 5;  // X4 MultiHub, File Browser, Library, File transfer, Settings\n",
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
        "  std::vector<const char*> menuItems = {tr(STR_BROWSE_FILES), tr(STR_LIBRARY), tr(STR_FILE_TRANSFER),\n"
        "                                        tr(STR_SETTINGS_TITLE)};\n"
        "  std::vector<UIIcon> menuIcons = {Folder, Library, Transfer, Settings};\n",
        "  std::vector<const char*> menuItems = {\"X4 MultiHub\", tr(STR_BROWSE_FILES), tr(STR_LIBRARY),\n"
        "                                        tr(STR_FILE_TRANSFER), tr(STR_SETTINGS_TITLE)};\n"
        "  std::vector<UIIcon> menuIcons = {Blocks, Folder, Library, Transfer, Settings};\n",
        "Home MultiHub menu",
    )
    replace_once(
        home_cpp,
        "    menuItems.insert(menuItems.begin() + 2, tr(STR_OPDS_BROWSER));\n"
        "    menuIcons.insert(menuIcons.begin() + 2, Blocks);\n",
        "    menuItems.insert(menuItems.begin() + 3, tr(STR_OPDS_BROWSER));\n"
        "    menuIcons.insert(menuIcons.begin() + 3, Blocks);\n",
        "Home OPDS insertion offset",
    )
    replace_once(
        home_cpp,
        "void HomeActivity::onFileBrowserOpen() { activityManager.goToFileBrowser(); }\n",
        "void HomeActivity::onMultiHubOpen() { activityManager.goToMultiHub(); }\n\n"
        "void HomeActivity::onFileBrowserOpen() { activityManager.goToFileBrowser(); }\n",
        "Home MultiHub implementation",
    )

    # Cover Grid theme is available on PSRAM-capable X4C. It needs the same
    # extra MultiHub tab or menu indexes would diverge from HomeActivity.
    replace_once(
        grid_h,
        "  std::array<freeink::ui::TabItem, 5> tabItems;\n",
        "  std::array<freeink::ui::TabItem, 6> tabItems;\n",
        "CoverGrid tab capacity",
    )
    replace_once(
        grid_cpp,
        "  static constexpr const uint8_t* ICONS[] = {FolderIcon, LibraryIcon, BlocksIcon, TransferIcon, Settings2Icon};\n"
        "  int count = 0;\n"
        "  for (int i = 0; i < 5; ++i) {\n"
        "    if (i == 2 && !hasOpds) continue;\n",
        "  static constexpr const uint8_t* ICONS[] = {BlocksIcon, FolderIcon, LibraryIcon, BlocksIcon, TransferIcon,\n"
        "                                             Settings2Icon};\n"
        "  int count = 0;\n"
        "  for (int i = 0; i < 6; ++i) {\n"
        "    if (i == 3 && !hasOpds) continue;\n",
        "CoverGrid MultiHub tab order",
    )
    replace_once(
        grid_cpp,
        "    const int icon = !self.hasOpds && index >= 2 ? index + 1 : index;\n",
        "    const int icon = !self.hasOpds && index >= 3 ? index + 1 : index;\n",
        "CoverGrid compressed icon mapping",
    )

    # Copy the complete MultiHub feature overlay unchanged. This preserves the
    # user's Reader, Field Manual, Planner, Markets, Weather, News, Dashboard,
    # Power/NTP, Gateway auth and Diagnostics assumptions.
    dest = repo / "src/multihub"
    if dest.exists():
        raise PatchError(f"{dest} already exists; refusing to overwrite")
    shutil.copytree(overlay / "src/multihub", dest)

    # Make the visible CrossPoint version string unambiguous for the hardware
    # test candidate while still using the official X4C release environment.
    patch_version_label(platformio)

    print("X4 MultiHub v1.7 X4C/ESP32-S3 patch applied to CrossPoint 1.6.5.")


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
