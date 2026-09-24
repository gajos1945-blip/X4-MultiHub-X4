X4 MultiHub News Terminal v1.1-dev

Runtime files:
  /.x4-multihub/news/feeds.jsonl
  /.x4-multihub/news/favorites.jsonl
  /.x4-multihub/news/current_article.txt

Zrodla dodajesz bezposrednio na X4:
1. Wiadomosci
2. Dodaj zrodlo RSS/Atom
3. wpisz nazwe
4. wpisz publiczny URL RSS lub Atom

Dlugie przytrzymanie zrodla:
  usun zrodlo

W kanale:
  klik artykulu -> zapis streszczenia/tresci dostarczonej przez RSS/Atom
                   do current_article.txt i otwarcie w readerze CrossPoint
  dlugie przytrzymanie -> dodaj/usun Ulubione

W v1.1-dev gateway NIE pobiera calej strony WWW artykulu.
Czytana jest tresc/summary dostarczona przez sam kanal RSS/Atom.

Bezpieczenstwo gateway:
- http/https only,
- brak credentials w URL,
- blokada prywatnych/loopback/link-local/reserved IP,
- ponowna walidacja redirectow,
- max 1 MiB feed,
- max 20 artykulow odpowiedzi.
