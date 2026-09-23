from pathlib import Path
import sys
import urllib.error
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "gateway"))

from multihub_gateway.http_client import JsonHttpClient, HttpError

class FakeResponse:
    def __init__(self, payload=b'{"ok": true}'):
        self.payload = payload
    def __enter__(self):
        return self
    def __exit__(self, *args):
        return False
    def read(self):
        return self.payload

def test_retry_after_transient_connection_error():
    calls = [OSError("temporary"), FakeResponse()]
    def fake_urlopen(*args, **kwargs):
        item = calls.pop(0)
        if isinstance(item, Exception):
            raise item
        return item

    with patch("urllib.request.urlopen", side_effect=fake_urlopen), \
         patch("time.sleep") as sleep:
        data = JsonHttpClient().get_json("https://example.invalid", attempts=2)

    assert data == {"ok": True}
    assert sleep.call_count == 1

def test_non_retryable_http_400_fails_immediately():
    error = urllib.error.HTTPError(
        "https://example.invalid", 400, "bad", hdrs=None, fp=None
    )
    with patch("urllib.request.urlopen", side_effect=error), \
         patch("time.sleep") as sleep:
        try:
            JsonHttpClient().get_json("https://example.invalid", attempts=3)
            assert False, "HTTP 400 should not retry"
        except HttpError:
            pass
    assert sleep.call_count == 0
