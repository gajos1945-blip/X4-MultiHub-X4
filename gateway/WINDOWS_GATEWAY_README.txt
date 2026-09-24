X4 Data Gateway 1.7 — Windows

Artifact:
  X4DataGateway.exe

Uruchomienie:
1. Uruchom X4DataGateway.exe.
2. Pozostaw port 8788 albo wybierz inny wolny port.
3. Wklej EODHD API token, jesli chcesz korzystac z Rynkow.
4. Dla dodatkowej kontroli dostepu kliknij "Generuj" przy Gateway access token.
5. Kliknij "Zapisz ustawienia".
6. Kliknij START.
7. W polu "Adres dla X4" zobaczysz np.:
     http://192.168.1.25:8788
8. Ten adres wpisz na X4:
     Ustawienia -> X4 Data Gateway
9. Jesli Gateway access token jest ustawiony, wpisz identyczny token na X4:
     Ustawienia -> Gateway access token
10. Kliknij "Sprawdz /health".

Gateway access token:
- jest opcjonalny,
- jesli ustawiony, wszystkie endpointy /v1/* wymagaja naglowka X-X4-Token,
- /health pozostaje publiczne i pokazuje tylko auth_required=true/false,
- token nie jest zwracany przez /health ani logowany przez gateway,
- przycisk "Pokaz / kopiuj" ujawnia token tylko na jawne zadanie uzytkownika.

Bezpieczenstwo:
- EODHD token nie trafia do firmware,
- EODHD token nie trafia do repo GitHub,
- Gateway access token nie jest kompilowany do firmware; X4 zapisuje go na microSD,
- oba sekrety zapisane w aplikacji Windows sa chronione DPAPI dla biezacego uzytkownika,
- RSS/Atom nadal blokuje prywatne/loopback/link-local/reserved targety,
  ponownie waliduje redirecty, ma limit 1 MiB i max 20 artykulow.

Wazne ograniczenie:
- X4 laczy sie z gateway przez HTTP w LAN,
- access token ogranicza dostep, ale NIE szyfruje ruchu,
- uzywaj tylko w zaufanej sieci prywatnej,
- nie wystawiaj portu gateway bezposrednio do Internetu.

Windows Firewall moze przy pierwszym uruchomieniu poprosic o zgode.
Zezwalaj tylko dla zaufanej sieci prywatnej/LAN.


Diagnostyka v1.7:
- `GET /v1/ping` jest lokalnym testem auth i nie wywoluje zewnetrznych providerow.
