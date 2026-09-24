from __future__ import annotations

import ipaddress
import queue
import secrets
import socket
import threading
import tkinter as tk
from tkinter import messagebox, ttk
import urllib.request
import webbrowser

from multihub_gateway.config import GatewayConfig
from multihub_gateway.server_runtime import create_http_server
from multihub_gateway.windows_config import (
    WindowsGatewaySettings,
    load_settings,
    save_settings,
)


APP_TITLE = "X4 Data Gateway 1.6"


def discover_lan_ipv4() -> list[str]:
    values: set[str] = set()

    try:
        for item in socket.getaddrinfo(socket.gethostname(), None, socket.AF_INET):
            values.add(item[4][0])
    except OSError:
        pass

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            sock.connect(("8.8.8.8", 80))
            values.add(sock.getsockname()[0])
        finally:
            sock.close()
    except OSError:
        pass

    result: list[str] = []
    for raw in values:
        try:
            addr = ipaddress.ip_address(raw)
        except ValueError:
            continue
        if addr.version == 4 and not addr.is_loopback and not addr.is_link_local:
            result.append(raw)

    return sorted(result)


def valid_access_token(value: str) -> bool:
    if not value:
        return True
    if not (8 <= len(value) <= 96):
        return False
    return all(0x21 <= ord(ch) <= 0x7E for ch in value)


class GatewayWindow:
    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        self.root.title(APP_TITLE)
        self.root.minsize(760, 610)

        self.server = None
        self.thread: threading.Thread | None = None
        self.logs: queue.Queue[str] = queue.Queue()

        try:
            settings = load_settings()
        except Exception as exc:
            settings = WindowsGatewaySettings()
            messagebox.showwarning(
                APP_TITLE,
                f"Nie udalo sie odczytac zapisanych ustawien:\n{exc}",
            )

        self.port_var = tk.StringVar(value=str(settings.port))
        self.token_var = tk.StringVar(value=settings.eodhd_token)
        self.access_token_var = tk.StringVar(value=settings.access_token)
        self.status_var = tk.StringVar(value="STOPPED")
        self.url_var = tk.StringVar(value=self._best_url(settings.port))

        self._build_ui()
        self._schedule_log_pump()
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

    def _best_url(self, port: int) -> str:
        addresses = discover_lan_ipv4()
        host = addresses[0] if addresses else "127.0.0.1"
        return f"http://{host}:{port}"

    def _build_ui(self) -> None:
        pad = {"padx": 10, "pady": 6}

        outer = ttk.Frame(self.root)
        outer.pack(fill="both", expand=True, padx=14, pady=14)

        ttk.Label(
            outer,
            text="X4 Data Gateway",
            font=("Segoe UI", 18, "bold"),
        ).pack(anchor="w")

        ttk.Label(
            outer,
            text="Gateway LAN dla X4 MultiHub: Rynki, Pogoda i Wiadomosci.",
        ).pack(anchor="w", pady=(0, 14))

        form = ttk.Frame(outer)
        form.pack(fill="x")

        ttk.Label(form, text="Port:").grid(row=0, column=0, sticky="w", **pad)
        ttk.Entry(form, width=12, textvariable=self.port_var).grid(
            row=0, column=1, sticky="w", **pad
        )

        ttk.Label(form, text="EODHD API token:").grid(
            row=1, column=0, sticky="w", **pad
        )
        ttk.Entry(
            form, width=58, textvariable=self.token_var, show="*"
        ).grid(row=1, column=1, columnspan=3, sticky="ew", **pad)

        ttk.Label(form, text="Gateway access token:").grid(
            row=2, column=0, sticky="w", **pad
        )
        ttk.Entry(
            form, width=40, textvariable=self.access_token_var, show="*"
        ).grid(row=2, column=1, sticky="ew", **pad)
        ttk.Button(
            form, text="Generuj", command=self.generate_access_token
        ).grid(row=2, column=2, sticky="w", **pad)
        ttk.Button(
            form, text="Pokaz / kopiuj", command=self.show_copy_access_token
        ).grid(row=2, column=3, sticky="w", **pad)

        form.columnconfigure(1, weight=1)

        ttk.Label(
            outer,
            text=(
                "EODHD i Gateway token sa zapisywane lokalnie przez Windows DPAPI. "
                "Gateway token jest opcjonalny; po ustawieniu wpisz ten sam token na X4."
            ),
        ).pack(anchor="w", pady=(0, 4))

        ttk.Label(
            outer,
            text=(
                "Uwaga: token ogranicza dostep do API, ale HTTP w LAN nie szyfruje ruchu. "
                "Uzywaj tylko w zaufanej sieci prywatnej."
            ),
        ).pack(anchor="w", pady=(0, 10))

        controls = ttk.Frame(outer)
        controls.pack(fill="x", pady=(2, 10))

        ttk.Button(controls, text="START", command=self.start).pack(
            side="left", padx=(0, 8)
        )
        ttk.Button(controls, text="STOP", command=self.stop).pack(
            side="left", padx=(0, 8)
        )
        ttk.Button(controls, text="Zapisz ustawienia", command=self.save).pack(
            side="left", padx=(0, 8)
        )
        ttk.Button(controls, text="Sprawdz /health", command=self.health).pack(
            side="left", padx=(0, 8)
        )

        status = ttk.LabelFrame(outer, text="Status")
        status.pack(fill="x", pady=(0, 10))

        ttk.Label(status, text="Serwer:").grid(row=0, column=0, sticky="w", **pad)
        ttk.Label(status, textvariable=self.status_var).grid(
            row=0, column=1, sticky="w", **pad
        )

        ttk.Label(status, text="Adres dla X4:").grid(
            row=1, column=0, sticky="w", **pad
        )
        ttk.Entry(
            status, textvariable=self.url_var, state="readonly"
        ).grid(row=1, column=1, sticky="ew", **pad)
        ttk.Button(
            status,
            text="Otworz /health",
            command=lambda: webbrowser.open(self.url_var.get() + "/health"),
        ).grid(row=1, column=2, sticky="e", **pad)
        status.columnconfigure(1, weight=1)

        ttk.Label(
            outer,
            text=(
                "Na X4 wpisz adres w: Ustawienia -> X4 Data Gateway, "
                "a token w: Ustawienia -> Gateway access token."
            ),
        ).pack(anchor="w", pady=(0, 8))

        logs_frame = ttk.LabelFrame(outer, text="Log")
        logs_frame.pack(fill="both", expand=True)

        self.log_text = tk.Text(logs_frame, height=12, wrap="word", state="disabled")
        self.log_text.pack(fill="both", expand=True, padx=8, pady=8)

    def _log(self, value: str) -> None:
        self.logs.put(value)

    def _schedule_log_pump(self) -> None:
        while True:
            try:
                line = self.logs.get_nowait()
            except queue.Empty:
                break
            self.log_text.configure(state="normal")
            self.log_text.insert("end", line + "\n")
            self.log_text.see("end")
            self.log_text.configure(state="disabled")
        self.root.after(150, self._schedule_log_pump)

    def _port(self) -> int:
        value = int(self.port_var.get().strip())
        if not (1 <= value <= 65535):
            raise ValueError("Port musi byc w zakresie 1..65535")
        return value

    def _access_token(self) -> str:
        value = self.access_token_var.get().strip()
        if not valid_access_token(value):
            raise ValueError(
                "Gateway access token: 8-96 drukowalnych znakow ASCII bez spacji"
            )
        return value

    def generate_access_token(self) -> None:
        # 64-bit random token, 16 lowercase hex characters: reasonably strong for
        # trusted-LAN access control while still practical to type on the X4.
        value = secrets.token_hex(8)
        self.access_token_var.set(value)
        self._log("Wygenerowano nowy Gateway access token. Zapisz ustawienia.")

    def show_copy_access_token(self) -> None:
        try:
            value = self._access_token()
        except Exception as exc:
            messagebox.showerror(APP_TITLE, str(exc))
            return
        if not value:
            messagebox.showinfo(APP_TITLE, "Gateway access token jest pusty.")
            return

        self.root.clipboard_clear()
        self.root.clipboard_append(value)
        self.root.update()
        messagebox.showinfo(
            APP_TITLE,
            "Gateway access token (skopiowany do schowka):\n\n" + value,
        )

    def save(self) -> None:
        try:
            settings = WindowsGatewaySettings(
                port=self._port(),
                eodhd_token=self.token_var.get().strip(),
                access_token=self._access_token(),
            )
            target = save_settings(settings)
            self.url_var.set(self._best_url(settings.port))
            self._log(f"Ustawienia zapisane: {target}")
        except Exception as exc:
            messagebox.showerror(APP_TITLE, str(exc))

    def start(self) -> None:
        if self.server is not None:
            self._log("Gateway juz dziala.")
            return

        try:
            port = self._port()
            provider_token = self.token_var.get().strip()
            access_token = self._access_token()
            config = GatewayConfig(
                host="0.0.0.0",
                port=port,
                eodhd_token=provider_token,
                access_token=access_token,
            )
            self.server = create_http_server(config, self._log)
        except Exception as exc:
            self.server = None
            messagebox.showerror(APP_TITLE, f"Nie mozna uruchomic gateway:\n{exc}")
            return

        self.thread = threading.Thread(
            target=self.server.serve_forever,
            name="X4DataGateway",
            daemon=True,
        )
        self.thread.start()

        self.status_var.set("RUNNING")
        self.url_var.set(self._best_url(port))
        self._log(f"START {self.url_var.get()}")
        self._log(
            "EODHD: " + ("CONFIGURED" if provider_token else "NOT CONFIGURED")
        )
        self._log(
            "AUTH: " + ("REQUIRED" if access_token else "DISABLED")
        )

    def stop(self) -> None:
        server = self.server
        if server is None:
            self.status_var.set("STOPPED")
            return

        self.server = None
        self.status_var.set("STOPPING...")
        try:
            server.shutdown()
            server.server_close()
        finally:
            self.status_var.set("STOPPED")
            self._log("STOP")

    def health(self) -> None:
        url = self.url_var.get().rstrip("/") + "/health"

        def worker() -> None:
            try:
                with urllib.request.urlopen(url, timeout=3.0) as response:
                    body = response.read(4096).decode("utf-8", errors="replace")
                self._log(f"HEALTH {response.status}: {body}")
            except Exception as exc:
                self._log(f"HEALTH ERROR: {exc}")

        threading.Thread(target=worker, daemon=True).start()

    def on_close(self) -> None:
        self.stop()
        self.root.destroy()


def main() -> None:
    root = tk.Tk()
    GatewayWindow(root)
    root.mainloop()


if __name__ == "__main__":
    main()
