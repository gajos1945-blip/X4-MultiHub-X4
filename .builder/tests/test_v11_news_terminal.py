from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / ".builder/overlay/src/multihub"

def test_news_sources_exist():
    for name in (
        "NewsStore.h", "NewsStore.cpp",
        "NewsGatewayClient.h", "NewsGatewayClient.cpp",
        "NewsActivity.h", "NewsActivity.cpp",
        "NewsFeedActivity.h", "NewsFeedActivity.cpp",
        "NewsFavoritesActivity.h", "NewsFavoritesActivity.cpp",
    ):
        assert (OVERLAY / name).is_file()

def test_multihub_opens_news_terminal():
    source = (OVERLAY / "MultiHubActivity.cpp").read_text(encoding="utf-8")
    assert '#include "NewsActivity.h"' in source
    assert '"Wiadomosci"' in source
    assert '"RSS / Atom / Ulubione"' in source
    assert "std::make_unique<NewsActivity>" in source

def test_news_store_paths_and_caps_are_explicit():
    header = (OVERLAY / "NewsStore.h").read_text(encoding="utf-8")
    assert 'FEEDS_PATH = "/.x4-multihub/news/feeds.jsonl"' in header
    assert 'FAVORITES_PATH = "/.x4-multihub/news/favorites.jsonl"' in header
    assert 'ARTICLE_PATH = "/.x4-multihub/news/current_article.txt"' in header
    assert "MAX_FEEDS = 30" in header
    assert "MAX_FAVORITES = 100" in header

def test_news_gateway_client_uses_multihub_gateway_not_direct_internet_provider():
    source = (OVERLAY / "NewsGatewayClient.cpp").read_text(encoding="utf-8")
    assert "/v1/news?url=" in source
    assert "WiFi.status() != WL_CONNECTED" in source
    assert "rss" not in source.lower() or "RSS/Atom" in source
    assert "eodhd.com" not in source.lower()
    assert "open-meteo.com" not in source.lower()

def test_news_article_opens_through_crosspoint_reader():
    source = (OVERLAY / "NewsFeedActivity.cpp").read_text(encoding="utf-8")
    assert "NewsStore::writeArticleText" in source
    assert "activityManager.goToReader(NewsStore::ARTICLE_PATH)" in source
    assert "toggleFavorite" in source

def test_news_gateway_has_ssrf_and_size_guards():
    source = (ROOT / "gateway/multihub_gateway/providers/rss.py").read_text(encoding="utf-8")
    assert "address.is_global" in source
    assert "_SafeRedirectHandler" in source
    assert "MAX_FEED_BYTES = 1024 * 1024" in source
    assert "MAX_ITEMS = 20" in source

def test_v11_manifest_is_truthful():
    data = json.loads((ROOT / ".builder/manifest.json").read_text(encoding="utf-8"))
    assert data["version"] == "1.6-dev"
    assert any("News Terminal main screen" in x for x in data["implemented"])
    assert "Full web-page readability/article extraction beyond feed-provided summary/content" in data["not_implemented"]
    assert any("Power Manager settings screen" in x for x in data["implemented"])
    assert data["physical_device_verified"] is False

def test_v11_workflow_is_dev_not_final_release():
    workflow = (ROOT / ".github/workflows/build-x4-bin.yml").read_text(encoding="utf-8")
    assert "BUILD X4 MULTIHUB v1.6 DEV BIN" in workflow
    assert "X4_MultiHub_X4_v1_6_dev" in workflow
    assert "X4_MultiHub_X4_v1.6-dev.bin" in workflow
    assert "Custom.bin" not in workflow

def test_release_guard_requires_news_runtime_markers():
    guard = (ROOT / ".builder/tools/release_guard.py").read_text(encoding="utf-8")
    for marker in (
        "Wiadomosci",
        "RSS / Atom / Ulubione",
        "News Terminal",
        "Dodaj zrodlo RSS/Atom",
        "Ulubione wiadomosci",
        "Odswiez kanal",
        "URL RSS/Atom http(s)://",
    ):
        assert marker in guard
