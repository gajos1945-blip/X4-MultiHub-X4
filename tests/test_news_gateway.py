from __future__ import annotations

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "gateway"))

from multihub_gateway.providers.rss import RssProvider, _validate_public_url


RSS = b"""<?xml version="1.0" encoding="UTF-8"?>
<rss version="2.0">
  <channel>
    <title>Przykladowe Wiadomosci</title>
    <item>
      <title>Test pierwszy</title>
      <link>https://example.com/a</link>
      <pubDate>Wed, 23 Sep 2026 10:00:00 GMT</pubDate>
      <description><![CDATA[<p>Akapit <b>pierwszy</b>.</p>]]></description>
    </item>
    <item>
      <title>Test drugi</title>
      <link>https://example.com/b</link>
      <description>Drugi opis</description>
    </item>
  </channel>
</rss>
"""

ATOM = b"""<?xml version="1.0" encoding="utf-8"?>
<feed xmlns="http://www.w3.org/2005/Atom">
  <title>Atom Test</title>
  <entry>
    <title>Atom artykul</title>
    <link href="https://example.org/news/1"/>
    <updated>2026-09-23T11:30:00Z</updated>
    <summary type="html">&lt;p&gt;Atom &lt;b&gt;summary&lt;/b&gt;&lt;/p&gt;</summary>
  </entry>
</feed>
"""


def test_rss_parser_extracts_compact_articles_and_strips_html():
    result = RssProvider().parse_bytes(RSS, "https://example.com/feed.xml", 15)
    assert result.title == "Przykladowe Wiadomosci"
    assert len(result.articles) == 2
    assert result.articles[0].title == "Test pierwszy"
    assert result.articles[0].link == "https://example.com/a"
    assert result.articles[0].summary == "Akapit pierwszy."
    assert result.articles[0].source == "Przykladowe Wiadomosci"


def test_atom_parser_supports_href_links():
    result = RssProvider().parse_bytes(ATOM, "https://example.org/atom.xml", 15)
    assert result.title == "Atom Test"
    assert len(result.articles) == 1
    assert result.articles[0].link == "https://example.org/news/1"
    assert result.articles[0].summary == "Atom summary"


def test_feed_limit_is_enforced():
    result = RssProvider().parse_bytes(RSS, "https://example.com/feed.xml", 1)
    assert len(result.articles) == 1


def test_private_and_loopback_feed_targets_are_rejected():
    for url in (
        "http://127.0.0.1/feed",
        "http://10.0.0.1/feed",
        "http://192.168.1.2/feed",
        "http://169.254.1.1/feed",
    ):
        try:
            _validate_public_url(url)
            assert False, f"should reject {url}"
        except ValueError:
            pass


def test_feed_credentials_are_rejected_before_network_access():
    try:
        _validate_public_url("https://user:secret@example.com/feed.xml")
        assert False, "credentials must be rejected"
    except ValueError:
        pass
