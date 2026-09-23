from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STORE = ROOT / ".builder/overlay/src/multihub/MarketStore.cpp"

def test_market_store_does_not_use_unsupported_halfile_readstring():
    source = STORE.read_text(encoding="utf-8")
    assert ".readString()" not in source
    assert "while (file.available()" in source
    assert "file.read()" in source

def test_market_gateway_config_read_is_bounded():
    source = STORE.read_text(encoding="utf-8")
    assert "raw.size() < 2048" in source
