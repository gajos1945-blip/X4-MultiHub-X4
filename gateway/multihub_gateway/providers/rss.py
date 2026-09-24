from __future__ import annotations

from dataclasses import dataclass
from html import unescape
from html.parser import HTMLParser
import ipaddress
import re
import socket
import urllib.error
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET


MAX_FEED_BYTES = 1024 * 1024
MAX_ITEMS = 20
MAX_SUMMARY_CHARS = 1600


@dataclass(frozen=True)
class NewsArticle:
    title: str
    link: str
    source: str
    published: str
    summary: str


@dataclass(frozen=True)
class NewsFeedResult:
    title: str
    url: str
    articles: list[NewsArticle]


class _TextExtractor(HTMLParser):
    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.parts: list[str] = []

    def handle_data(self, data: str) -> None:
        value = " ".join(data.split())
        if value:
            self.parts.append(value)

    def text(self) -> str:
        return " ".join(self.parts)


def _strip_html(value: str) -> str:
    parser = _TextExtractor()
    try:
        parser.feed(value)
        parser.close()
        text = " ".join(unescape(parser.text()).split())
    except Exception:
        text = " ".join(unescape(value).split())

    # HTML tag boundaries can introduce a synthetic space before punctuation.
    return re.sub(r"\s+([,.;:!?])", r"\1", text)


def _local(tag: str) -> str:
    if "}" in tag:
        tag = tag.rsplit("}", 1)[1]
    if ":" in tag:
        tag = tag.rsplit(":", 1)[1]
    return tag.lower()


def _first_text(node: ET.Element, names: set[str]) -> str:
    for child in node.iter():
        if _local(child.tag) in names and child.text:
            value = " ".join(child.text.split())
            if value:
                return value
    return ""


def _entry_link(node: ET.Element, base_url: str) -> str:
    # Atom <link href="..."> first.
    for child in node:
        if _local(child.tag) != "link":
            continue
        href = (child.attrib.get("href") or "").strip()
        rel = (child.attrib.get("rel") or "alternate").strip().lower()
        if href and rel in {"", "alternate"}:
            return urllib.parse.urljoin(base_url, href)

    # RSS <link>text</link>.
    for child in node:
        if _local(child.tag) == "link" and child.text:
            return urllib.parse.urljoin(base_url, child.text.strip())

    return ""


def _validate_public_url(url: str) -> None:
    parsed = urllib.parse.urlsplit(url)
    if parsed.scheme not in {"http", "https"}:
        raise ValueError("RSS/Atom URL must use http or https")
    if not parsed.hostname:
        raise ValueError("RSS/Atom URL has no hostname")
    if parsed.username or parsed.password:
        raise ValueError("credentials in RSS/Atom URL are not allowed")

    try:
        port = parsed.port
    except ValueError as exc:
        raise ValueError("invalid RSS/Atom port") from exc
    if port is not None and not (1 <= port <= 65535):
        raise ValueError("invalid RSS/Atom port")

    try:
        infos = socket.getaddrinfo(parsed.hostname, port or (443 if parsed.scheme == "https" else 80))
    except OSError as exc:
        raise RuntimeError(f"RSS/Atom DNS error: {exc}") from exc

    addresses = {info[4][0] for info in infos if info and info[4]}
    if not addresses:
        raise RuntimeError("RSS/Atom hostname resolved to no address")

    for raw in addresses:
        try:
            address = ipaddress.ip_address(raw.split("%", 1)[0])
        except ValueError as exc:
            raise RuntimeError("RSS/Atom resolved to invalid IP") from exc
        if not address.is_global:
            raise ValueError("RSS/Atom URL resolves to non-public address")


class _SafeRedirectHandler(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        absolute = urllib.parse.urljoin(req.full_url, newurl)
        _validate_public_url(absolute)
        return super().redirect_request(req, fp, code, msg, headers, absolute)


class RssProvider:
    def parse_bytes(self, raw: bytes, source_url: str, limit: int = 15) -> NewsFeedResult:
        if not raw:
            raise ValueError("empty RSS/Atom response")
        if len(raw) > MAX_FEED_BYTES:
            raise ValueError("RSS/Atom response is too large")

        try:
            root = ET.fromstring(raw)
        except ET.ParseError as exc:
            raise ValueError("invalid RSS/Atom XML") from exc

        limit = max(1, min(int(limit), MAX_ITEMS))

        feed_title = ""
        for node in root.iter():
            local = _local(node.tag)
            if local in {"channel", "feed"}:
                feed_title = _first_text(node, {"title"})
                if feed_title:
                    break
        if not feed_title:
            feed_title = _first_text(root, {"title"})
        feed_title = feed_title[:120] if feed_title else "RSS/Atom"

        entries: list[ET.Element] = []
        for node in root.iter():
            if _local(node.tag) in {"item", "entry"}:
                entries.append(node)

        articles: list[NewsArticle] = []
        for entry in entries[:limit]:
            title = _first_text(entry, {"title"})[:240]
            link = _entry_link(entry, source_url)
            published = _first_text(
                entry, {"pubdate", "published", "updated", "date"}
            )[:120]

            raw_summary = _first_text(
                entry, {"description", "summary", "content", "encoded"}
            )
            summary = _strip_html(raw_summary)[:MAX_SUMMARY_CHARS]

            if not title or not link:
                continue
            parsed_link = urllib.parse.urlsplit(link)
            if parsed_link.scheme not in {"http", "https"}:
                continue

            articles.append(
                NewsArticle(
                    title=title,
                    link=link[:700],
                    source=feed_title,
                    published=published,
                    summary=summary,
                )
            )

        return NewsFeedResult(
            title=feed_title,
            url=source_url,
            articles=articles,
        )

    def fetch(self, url: str, limit: int = 15) -> NewsFeedResult:
        _validate_public_url(url)

        opener = urllib.request.build_opener(_SafeRedirectHandler())
        req = urllib.request.Request(
            url,
            method="GET",
            headers={
                "User-Agent": "X4-MultiHub-Gateway/1.1",
                "Accept": "application/rss+xml, application/atom+xml, application/xml, text/xml, */*;q=0.5",
                "Accept-Encoding": "identity",
            },
        )

        try:
            with opener.open(req, timeout=20.0) as response:
                raw = response.read(MAX_FEED_BYTES + 1)
                final_url = response.geturl()
        except urllib.error.HTTPError as exc:
            detail = exc.read(300).decode("utf-8", errors="replace")
            raise RuntimeError(f"RSS/Atom HTTP {exc.code}: {detail}") from exc
        except OSError as exc:
            raise RuntimeError(f"RSS/Atom connection error: {exc}") from exc

        if len(raw) > MAX_FEED_BYTES:
            raise ValueError("RSS/Atom response is too large")

        _validate_public_url(final_url)
        return self.parse_bytes(raw, final_url, limit)
