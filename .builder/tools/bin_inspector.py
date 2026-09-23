from __future__ import annotations

from pathlib import Path
import hashlib
import struct
from typing import Any

ESP_MAGIC = 0xE9
IMAGE_HEADER_SIZE = 24
SEGMENT_HEADER_SIZE = 8
CHECKSUM_SEED = 0xEF


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for block in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def inspect_bin(path: Path) -> dict[str, Any]:
    data = path.read_bytes()
    out: dict[str, Any] = {
        "path": str(path),
        "size": len(data),
        "sha256": sha256_file(path),
        "esp_image": False,
        "structure_valid": False,
        "segments": None,
        "entry_point": None,
        "chip_id_raw": None,
        "hash_appended": None,
        "segment_headers": [],
        "image_checksum_stored": None,
        "image_checksum_calculated": None,
        "image_checksum_valid": None,
        "appended_sha256_stored": None,
        "appended_sha256_calculated": None,
        "appended_sha256_valid": None,
        "trailing_bytes": None,
        "errors": [],
    }

    if len(data) < IMAGE_HEADER_SIZE:
        out["errors"].append("image shorter than ESP header")
        return out

    if data[0] != ESP_MAGIC:
        out["errors"].append(f"bad ESP magic 0x{data[0]:02X}")
        return out

    out["esp_image"] = True
    segment_count = int(data[1])
    out["segments"] = segment_count
    out["entry_point"] = f"0x{struct.unpack_from('<I', data, 4)[0]:08X}"
    out["chip_id_raw"] = int(struct.unpack_from("<H", data, 12)[0])
    out["hash_appended"] = bool(data[23])

    if segment_count <= 0 or segment_count > 16:
        out["errors"].append(f"invalid segment count {segment_count}")
        return out

    pos = IMAGE_HEADER_SIZE
    checksum = CHECKSUM_SEED
    segment_headers: list[dict[str, Any]] = []

    for index in range(segment_count):
        if pos + SEGMENT_HEADER_SIZE > len(data):
            out["errors"].append(f"segment {index} header out of range")
            return out

        load_addr, length = struct.unpack_from("<II", data, pos)
        pos += SEGMENT_HEADER_SIZE

        if length < 0 or pos + length > len(data):
            out["errors"].append(f"segment {index} data out of range")
            return out

        segment = data[pos:pos + length]
        for value in segment:
            checksum ^= value

        segment_headers.append({
            "index": index,
            "load_address": f"0x{load_addr:08X}",
            "length": int(length),
        })
        pos += length

    out["segment_headers"] = segment_headers

    # ESP checksum is placed at the final byte of a 16-byte-aligned block.
    prehash_end = ((pos + 1 + 15) // 16) * 16
    if prehash_end > len(data):
        out["errors"].append("checksum/alignment block out of range")
        return out

    stored_checksum = int(data[prehash_end - 1])
    out["image_checksum_stored"] = stored_checksum
    out["image_checksum_calculated"] = checksum
    out["image_checksum_valid"] = stored_checksum == checksum

    end = prehash_end
    if out["hash_appended"]:
        if end + 32 > len(data):
            out["errors"].append("appended SHA-256 flag set but digest is missing")
            return out

        stored_digest = data[end:end + 32]
        calculated_digest = hashlib.sha256(data[:end]).digest()
        out["appended_sha256_stored"] = stored_digest.hex()
        out["appended_sha256_calculated"] = calculated_digest.hex()
        out["appended_sha256_valid"] = stored_digest == calculated_digest
        end += 32
    else:
        out["appended_sha256_valid"] = None

    out["trailing_bytes"] = len(data) - end
    out["structure_valid"] = (
        not out["errors"]
        and out["image_checksum_valid"] is True
        and (not out["hash_appended"] or out["appended_sha256_valid"] is True)
        and out["trailing_bytes"] == 0
    )
    return out
