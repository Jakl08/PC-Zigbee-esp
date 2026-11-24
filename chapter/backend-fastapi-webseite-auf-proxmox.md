# Backend: FastAPI Webseite auf Proxmox (backend-fastapi-webseite-auf-proxmox.md)

Dieses Dokument beschreibt die Architektur, Verzeichnisstruktur und Funktionsweise der FastAPI-Webseite, die als zentraler Webserver und API-Schnittstelle auf einem Proxmox LXC-Container betrieben wird.

## 1. Verzeichnisstruktur auf dem Server

Das Projekt ist modular unter dem Verzeichnis `status_api/` [status_api](../files/LXC/status_api/) auf dem Server aufgebaut:

```text
status_api/
├── app/
│   ├── __pycache__/
│   ├── database.py       # SQLAlchemy / Datenbankverbindung (SQLite: status.db)
│   └── main.py           # FastAPI Haupteinstiegspunkt & App-Initialisierung
├── router/
│   ├── __pycache__/
│   └── api.py            # REST-Endpunkte (/get_status, /update_is_on, etc.)
├── static/
│   ├── index.html        # Frontend-Oberfläche (Dashboard)
│   ├── script.js         # Frontend Logik & Polling
│   └── style.css         # Styling der Weboberfläche
├── status.db             # Lokale SQLite-Datenbankdatei
└── venv/                 # Python Virtual Environment
```

## 2. Komponenten und Module

### A. Der Hauptserver (`app/main.py` & `app/database.py`)
*   **FastAPI Core:** Initialisiert die Webanwendung, bindet die statischen Dateien (`/static`) ein und registriert den API-Router aus dem `router/`-Verzeichnis.
*   **Datenbank-Anbindung (`database.py`):** Verwaltet die Verbindung zur lokalen SQLite-Datenbank (`status.db`) mittels SQLAlchemy, um den aktuellen System- und Schaltstatus persistent zu speichern.

### B. Die API-Schnittstelle (`router/api.py`)
*   Stellt die REST-Endpunkte bereit, die vom lokalen Hardware-Controller (Raspberry Pi / ESP32C6) sowie dem Frontend angesprochen werden:
    *   `GET /api/get_status/{device_name}`: Liefert den aktuellen Status zurück (inklusive `is_on` und des von der Website geforderten Status `is_requested`).
    *   `POST /api/update_is_on/{device_name}/{status}`: Aktualisiert den physischen Zustand oder verarbeitet Schaltwünsche.

### C. Das Web-Dashboard (`static/`)
*   **`index.html`:** Die grafische Benutzeroberfläche zur Visualisierung des PC-Zustands.
*   **`script.js` & `style.css`:** Übernehmen die asynchronen Abfragen (AJAX/Fetch) an den FastAPI-Server, um den Zustand in Echtzeit im Browser zu aktualisieren und Schaltbefehle auszulösen.

## 3. Betrieb und Deployment
*   Der Server läuft isoliert innerhalb eines Proxmox LXC-Containers.
*   Die Ausführung erfolgt innerhalb der isolierten Python-Umgebung (`venv/`), gesteuert über einen Prozessmanager oder direkt als Service, um die Erreichbarkeit unter der jeweils konfigurierten Domain sicherzustellen.