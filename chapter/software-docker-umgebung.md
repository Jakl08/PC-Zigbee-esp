# Software: Docker-Umgebung

Dieses Dokument beschreibt die Containerisierung des Python-Worker-Skripts für den PC-Power-Controller mittels Docker und Docker Compose. Das Setup stellt sicher, dass der Dienst im Hintergrund als Daemon läuft, bei Systemstarts automatisch neu startet und direkten Zugriff auf die Hardware-GPIOs des Raspberry Pi erhält.

<br><br>

## 1. Das Dockerfile (`pc-control/Dockerfile`)

Das `Dockerfile` basiert auf einem schlanken Python-Image und installiert alle notwendigen Kompilierungs- und Bibliotheksabhängigkeiten, um die GPIO-Schnittstelle anzusprechen.

### Inhalt des Dockerfiles
* **Basis-Image:** `python:3.9-slim`  – Ein ressourcenschonendes Debian-basiertes Python-Image.
* **Paketinstallation:** Aktualisiert die Paketquellen und installiert `gcc` sowie `python3-dev`, da diese für die Kompilierung hardwarenaher Python-Pakete benötigt werden. Danach wird der Paket-Cache bereinigt, um das Image klein zu halten.
* **Python-Abhängigkeiten:** Installiert die Module `RPi.GPIO` (für die Pin-Steuerung) und `requests` (für die REST-API-Kommunikation) ohne Pip-Cache.
* **Arbeitsverzeichnis & Code:** Setzt `/app` als Arbeitsverzeichnis und kopiert die lokale `main.py` in den Container.
* **Startbefehl:** Startet das Skript beim Hochfahren des Containers automatisch mit dem Befehl `python main.py`.

<br><br>

## 2. Die Docker Compose Konfiguration (`docker-compose.yml`)

Der Service wird in die bestehende `docker-compose.yml` integriert, um das Build-Verhalten, die Neustart-Richtlinien und vor allem die Hardware-Freigabe zu steuern.

### Erklärung der Konfigurationsparameter
* **`build: ./pc-control`:** Weist Docker an, das Image aus dem lokalen Unterordner `pc-control` (in dem sich das Dockerfile befindet) zu bauen.
* **`container_name: pc_controller`:** Vergibt einen festen, eindeutigen Namen für den Container.
* **`restart: unless-stopped`:** Sorgt dafür, dass der Container im Hintergrund läuft und bei einem Systemabsturz oder Neustart des Raspberry Pi automatisch wieder hochfährt.
* **`privileged: true`:** Gewährt dem Container erweiterte Systemrechte, um die hardwarenahe GPIO-Kommunikation fehlerfrei ausführen zu können.
* **`volumes: - /dev/gpiomem:/dev/gpiomem`:** Reicht den GPIO-Memory-Speicherbereich des Host-Systems direkt in den Container durch, damit die `RPi.GPIO`-Bibliothek physisch auf die Pins zugreifen kann, ohne direkt auf `/dev/mem` zugreifen zu müssen.

<br><br>

## 3. Deployment und Steuerung
Der Container wird im Verzeichnis der `docker-compose.yml` mit folgendem Befehl gebaut und im Hintergrund gestartet:
`docker compose up -d --build pc-controller`