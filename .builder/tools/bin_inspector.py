from __future__ import annotations
from pathlib import Path
import hashlib
import struct

def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for block in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()

def inspect_bin(path: Path):
    data = path.read_bytes()
    out = {
        "path": str(path),
        "size": len(data),
        "sha256": sha256_file(path),
        "esp_image": False,
        "segments": None,
        "entry_point": None,
    }
    if len(data) >= 24 and data[0] == 0xE9:
        out["esp_image"] = True
        out["segments"] = data[1]
        out["entry_point"] = f"0x{struct.unpack_from('<I', data, 4)[0]:08X}"
    return out
