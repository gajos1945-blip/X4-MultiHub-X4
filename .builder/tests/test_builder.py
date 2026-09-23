from __future__ import annotations
from pathlib import Path
import hashlib
import importlib.util
import json
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / ".builder" / "tools"
sys.path.insert(0, str(TOOLS))

from verify_project import parse_int, parse_partitions, verify_no_overlap, VerificationError
from bin_inspector import sha256_file

def test_manifest_has_no_fake_hardware_verification():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "0.3-dev"
    assert data["physical_device_verified"] is False
    assert data["automatic_erase"] is False

def test_partition_parser_units():
    assert parse_int("0x10000") == 65536
    assert parse_int("64K") == 65536
    assert parse_int("1M") == 1024 * 1024

def test_partition_parser_and_overlap_guard():
    with tempfile.TemporaryDirectory() as td:
        p = Path(td) / "partitions.csv"
        p.write_text(
            "app0,app,ota_0,0x10000,0x20000,\n"
            "app1,app,ota_1,0x30000,0x20000,\n",
            encoding="utf-8",
        )
        parts = parse_partitions(p)
        verify_no_overlap(parts)
        parts[1]["offset"] = 0x20000
        try:
            verify_no_overlap(parts)
            assert False, "overlap should be rejected"
        except VerificationError:
            pass

def test_sha256_helper():
    with tempfile.TemporaryDirectory() as td:
        p = Path(td) / "x.bin"
        p.write_bytes(b"abc")
        assert sha256_file(p) == hashlib.sha256(b"abc").hexdigest()

def test_release_markers_include_all_shell_modules():
    p = ROOT / ".builder/tools/release_guard.py"
    spec = importlib.util.spec_from_file_location("release_guard", p)
    mod = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(mod)
    markers = set(mod.REQUIRED_MARKERS)
    for marker in (
        b"X4 MultiHub",
        b"Field Manual",
        b"Daily Planner",
        b"Markets & Weather",
        b"Dashboard",
    ):
        assert marker in markers

def test_ci_blocks_unknown_and_partition_overflow():
    text = (ROOT / ".builder/build_ci.py").read_text(encoding="utf-8")
    assert 'report["flash_size"] == "UNKNOWN"' in text
    assert 'report["application_offset"] == "UNKNOWN"' in text
    assert "Application partition size UNKNOWN" in text
    assert 'info["size"] > int(app_partition["size"])' in text
    assert "merged_full_flash" in text

def test_patch_is_fail_closed():
    p = ROOT / ".builder/tools/patch_crosspoint.py"
    spec = importlib.util.spec_from_file_location("patch_crosspoint", p)
    mod = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(mod)

    with tempfile.TemporaryDirectory() as td:
        f = Path(td) / "x.txt"
        f.write_text("A\nONE\nB\n", encoding="utf-8")
        mod.replace_once(f, "ONE", "TWO", "single")
        assert "TWO" in f.read_text(encoding="utf-8")

        f.write_text("X\nX\n", encoding="utf-8")
        try:
            mod.replace_once(f, "X", "Y", "ambiguous")
            assert False, "ambiguous patch anchor should fail"
        except mod.PatchError:
            pass
