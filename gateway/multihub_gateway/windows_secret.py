from __future__ import annotations

import base64
import ctypes
from ctypes import wintypes
import sys


class SecretStoreError(RuntimeError):
    pass


class DATA_BLOB(ctypes.Structure):
    _fields_ = [
        ("cbData", wintypes.DWORD),
        ("pbData", ctypes.POINTER(ctypes.c_byte)),
    ]


def _blob(data: bytes) -> tuple[DATA_BLOB, ctypes.Array]:
    buffer = ctypes.create_string_buffer(data)
    blob = DATA_BLOB(
        len(data),
        ctypes.cast(buffer, ctypes.POINTER(ctypes.c_byte)),
    )
    return blob, buffer


def protect_text(value: str) -> str:
    if sys.platform != "win32":
        raise SecretStoreError("Windows DPAPI is available only on Windows")

    crypt32 = ctypes.windll.crypt32
    kernel32 = ctypes.windll.kernel32

    raw = value.encode("utf-8")
    in_blob, _keepalive = _blob(raw)
    out_blob = DATA_BLOB()

    ok = crypt32.CryptProtectData(
        ctypes.byref(in_blob),
        "X4 MultiHub Gateway",
        None,
        None,
        None,
        0,
        ctypes.byref(out_blob),
    )
    if not ok:
        raise SecretStoreError("CryptProtectData failed")

    try:
        protected = ctypes.string_at(out_blob.pbData, out_blob.cbData)
        return base64.b64encode(protected).decode("ascii")
    finally:
        kernel32.LocalFree(out_blob.pbData)


def unprotect_text(value: str) -> str:
    if sys.platform != "win32":
        raise SecretStoreError("Windows DPAPI is available only on Windows")

    crypt32 = ctypes.windll.crypt32
    kernel32 = ctypes.windll.kernel32

    try:
        raw = base64.b64decode(value.encode("ascii"), validate=True)
    except Exception as exc:
        raise SecretStoreError("Invalid encrypted secret") from exc

    in_blob, _keepalive = _blob(raw)
    out_blob = DATA_BLOB()

    ok = crypt32.CryptUnprotectData(
        ctypes.byref(in_blob),
        None,
        None,
        None,
        None,
        0,
        ctypes.byref(out_blob),
    )
    if not ok:
        raise SecretStoreError("CryptUnprotectData failed")

    try:
        clear = ctypes.string_at(out_blob.pbData, out_blob.cbData)
        return clear.decode("utf-8")
    finally:
        kernel32.LocalFree(out_blob.pbData)
