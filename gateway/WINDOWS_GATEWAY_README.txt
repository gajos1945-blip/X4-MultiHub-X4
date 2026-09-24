X4 Data Gateway 1.5 — Windows

Artifact:
  X4DataGateway.exe

Uruchomienie:
1. Uruchom X4DataGateway.exe.
2. Pozostaw port 8788 albo wybierz inny wolny port.
3. Wklej EODHD API token, jesli chcesz korzystac z Rynkow.
4. Kliknij "Zapisz ustawienia".
5. Kliknij START.
6. W polu "Adres dla X4" zobaczysz np.:
     http://192.168.1.25:8788
7. Ten adres wpisz na X4:
     Ustawienia -> X4 Data Gateway
8. Kliknij "Sprawdz /health".

Bezpieczenstwo:
- token EODHD nie trafia do firmware,
- token EODHD nie trafia do repo GitHub,
- zapisany token jest chroniony Windows DPAPI dla biezacego uzytkownika,
- /health pokazuje tylko czy EODHD jest skonfigurowany, nie ujawnia tokenu,
- RSS/Atom nadal ma blokade prywatnych/loopback/link-local/reserved targetow,
  walidacje redirectow, limit 1 MiB i max 20 artykulow.

Siec:
- gateway nasluchuje na 0.0.0.0, aby X4 w tej samej sieci LAN mogl go zobaczyc,
- Windows Firewall moze przy pierwszym uruchomieniu poprosic o zgode,
- zezwalaj tylko w zaufanej sieci prywatnej/LAN,
- nie wystawiaj portu gateway bezposrednio do Internetu.

Alternatywa bez EXE:
  START_GATEWAY_WINDOWS.cmd
wymaga Python 3.12+.
