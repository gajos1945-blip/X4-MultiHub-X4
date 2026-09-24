X4 MultiHub Gateway Access Control v1.6-dev

Runtime file:
  /.x4-multihub/gateway/auth.json

Ustawienie:
  Ustawienia -> Gateway access token

Token:
- 8..96 drukowalnych znakow ASCII bez spacji,
- pusty wpis usuwa token,
- wartosc tokenu nie jest pokazywana na liscie ustawien,
- token nie jest kompilowany do firmware.

Jesli Windows Gateway ma ustawiony token:
- ten sam token musi byc zapisany na X4,
- Markets / Weather / News dodaja naglowek X-X4-Token.

UWAGA:
Gateway nadal uzywa HTTP w LAN.
Token ogranicza dostep, ale nie szyfruje transmisji.
Uzywaj w zaufanej sieci prywatnej.
