from __future__ import annotations

from pathlib import Path
import configparser
import csv
import json


class VerificationError(RuntimeError):
    pass


def parse_int(value: str):
    v = value.strip()
    if not v:
        return None
    mult = 1
    if v[-1:].upper() == "K":
        mult, v = 1024, v[:-1]
    elif v[-1:].upper() == "M":
        mult, v = 1024 * 1024, v[:-1]
    return int(v, 0) * mult


def parse_partitions(path: Path):
    rows = []
    for raw in csv.reader(path.read_text(encoding="utf-8-sig").splitlines()):
        if not raw or not raw[0].strip() or raw[0].lstrip().startswith("#"):
            continue
        raw += [""] * (6 - len(raw))
        rows.append(
            {
                "name": raw[0].strip(),
                "type": raw[1].strip(),
                "subtype": raw[2].strip(),
                "offset": parse_int(raw[3]),
                "size": parse_int(raw[4]),
                "flags": raw[5].strip(),
            }
        )
    return rows


def verify_no_overlap(parts):
    known = sorted(
        (p for p in parts if p["offset"] is not None and p["size"] is not None),
        key=lambda p: p["offset"],
    )
    for a, b in zip(known, known[1:]):
        if a["offset"] + a["size"] > b["offset"]:
            raise VerificationError(f"Partition overlap: {a['name']} vs {b['name']}")


def load_ini(path: Path):
    cp = configparser.ConfigParser(
        interpolation=None,
        strict=False,
        inline_comment_prefixes=(";",),
    )
    cp.optionxform = str
    cp.read(path, encoding="utf-8")
    return cp


def resolve_section(cp: configparser.ConfigParser, section: str, stack=None):
    if stack is None:
        stack = []
    if section in stack:
        raise VerificationError("Circular PlatformIO extends: " + " -> ".join(stack + [section]))
    if not cp.has_section(section):
        raise VerificationError(f"Missing PlatformIO section: [{section}]")

    merged = {}
    raw = dict(cp.items(section, raw=True))
    extends = raw.get("extends", "")
    for parent in [x.strip() for x in extends.split(",") if x.strip()]:
        merged.update(resolve_section(cp, parent, stack + [section]))
    for key, value in raw.items():
        if key != "extends":
            merged[key] = value.strip()
    return merged


def inspect(root: Path, env: str = "gh_release"):
    ini = root / "platformio.ini"
    if not ini.is_file():
        raise VerificationError("platformio.ini missing")

    cp = load_ini(ini)
    section = f"env:{env}"
    cfg = resolve_section(cp, section)

    partition_name = cfg.get("board_build.partitions", "UNKNOWN")
    if partition_name == "UNKNOWN":
        raise VerificationError("board_build.partitions UNKNOWN")

    pfile = root / partition_name
    if not pfile.is_file():
        raise VerificationError(f"Partition file missing: {partition_name}")

    parts = parse_partitions(pfile)
    verify_no_overlap(parts)

    def get(name: str):
        return cfg.get(name, "UNKNOWN")

    return {
        "environment": env,
        "board": get("board"),
        "mcu": get("board_build.mcu"),
        "arduino_memory_type": get("board_build.arduino.memory_type"),
        "framework": get("framework"),
        "flash_size": get("board_upload.flash_size"),
        "maximum_size": get("board_upload.maximum_size"),
        "application_offset": get("board_upload.offset_address"),
        "partition_file": partition_name,
        "build_flags": get("build_flags"),
        "partitions": parts,
        "sources": {
            "environment": f"platformio.ini [{section}] with extends resolved",
            "flash_size": "platformio.ini",
            "application_offset": "platformio.ini",
            "partitions": partition_name,
        },
    }


def main():
    import argparse

    ap = argparse.ArgumentParser()
    ap.add_argument("repo")
    ap.add_argument("--env", default="x4c-gh_release")
    ap.add_argument("--json")
    ns = ap.parse_args()
    report = inspect(Path(ns.repo), ns.env)
    out = json.dumps(report, indent=2)
    if ns.json:
        Path(ns.json).write_text(out + "\n", encoding="utf-8")
    print(out)


if __name__ == "__main__":
    main()
