# Software: Python-Skript (software_python.md)

Das [main.py](../files/device-main.py) Skript ist nach dem prozeduralen Paradigma strukturiert und in funktionale Blöcke unterteilt, um Hardware-Events von der Netzwerk-Logik zu entkoppeln.

<br>

## 1. Initialisierung und Konfiguration
*   **Bibliotheken:** Nutzt `RPi.GPIO` für die Hardwaresteuerung und `requests` für REST-API-Aufrufe.
*   **Exception-Definition:** Eine benutzerdefinierte `HardwareChangeEvent` Klasse wird deklariert, um aus tiefen Schleifenebenen heraus in die Fehlerbehandlung zu springen.
*   **Lokale Konfiguration:** Die API-Adresse, der Gerätename, die GPIO-Pins und das Polling-Intervall werden aus `.env` geladen. `.env.example` dient als Vorlage und wird nach `.env` kopiert; die lokale `.env` wird nicht versioniert.
*   **GPIO-Setup:** Die Pins `RELAY_PIN` und `LED_PIN` werden aus der Konfiguration übernommen. Anschließend werden BCM-Modus, Warnungen und initiale Pin-Zustände gesetzt.

<br><br>

## 2. Hilfsfunktionen (Hardware & Netzwerk)
*   **`get_pc_status_phy()`:** Kapselt das physische Auslesen des GPIO 17 und gibt einen statischen Boolean zurück.
*   **`pulse_power_button()`:** Führt den physischen Schaltvorgang durch. Setzt den Relais-Pin für 0,5 Sekunden auf `LOW` und danach wieder auf `HIGH`.
*   **`report_status_to_api(is_on)`:** Setzt einen HTTP-POST-Request an die FastAPI-Infrastruktur ab, um physische Zustandsänderungen zu synchronisieren.
*   **`check_hardware_integrity(expected_state)`:** Vergleicht den durch das Argument übergebenen Soll-Zustand mit dem physischen Ist-Zustand (`get_pc_status_phy()`). Löst bei Diskrepanz die `HardwareChangeEvent` Exception aus.

<br><br>

## 3. Haupt-Pollling-Schleife (Logik-Kern)
Die Laufzeitumgebung ist in einer `while True:`-Schleife gekapselt, die von einem übergeordneten `try/except`-Block geschützt wird.

*   **Sub-Intervall-Überwachung:** Eine `for`-Schleife iteriert im Sekunden-Takt über `POLL_INTERVAL`. In jeder Iteration wird `check_hardware_integrity` aufgerufen. Dies gewährt sofortige Reaktionen auf manuelle Hardware-Eingriffe bei gleichzeitig ressourcenschonendem API-Polling.
*   **API-Synchronisation:** Nach Ablauf des Sub-Intervalls wird die FastAPI abgefragt (`is_requested`). 
*   **Schalt-Logik:** Weicht der API-Wunsch vom lokalen Zustand ab, wird das Relais getriggert. Im Anschluss folgt eine stabilisierende `for`-Schleife (max. 15 Sekunden), die den Ausführungsthread blockiert, bis der PC-Mainboard-Zustand sich physisch an den neuen logischen Zustand angepasst hat.

<br><br>

## 4. Interrupt-Handling
*   **`except HardwareChangeEvent`:** Fängt asynchrone Hardware-Änderungen (z.B. Benutzer drückt den Knopf am PC-Gehäuse) sauber ab. Ermittelt den neuen Ist-Zustand, korrigiert die lokale Tracking-Variable (`last_pc_status_phy`) und meldet die Änderung sofort per POST-Request an die FastAPI-Backend-Infrastruktur, um Netz- und Hardwarestatus zu synchronisieren.

<br><br>

## 5. Space und Time Complexity (Komplexitätsanalyse)

### Time Complexity (Zeitkomplexität): $O(1)$
Die Zeitkomplexität pro Logik-Durchlauf ist **konstant**. 
* **Hardware-Abfragen:** Die Funktionen `get_pc_status_phy()` und `check_hardware_integrity()` führen lediglich direkte I/O-Operationen am GPIO-Bus aus, welche in konstanter Zeit abgearbeitet werden.
* **Schleifen:** Die integrierte `for`-Schleife iteriert nicht über eine variable Eingabemenge oder Datenstruktur, sondern über den Skalarwert `POLL_INTERVAL` aus `.env`.
* **Netzwerk-Requests:** Die HTTP-Aufrufe (`requests.get` / `requests.post`) sind durch feste Timeouts (2 bzw. 3 Sekunden) serverseitig und clientseitig strikt begrenzt.
* Da das System nicht mit dynamischen Eingabegrößen (wie Listen oder Datenbankeinträgen) skaliert, bleibt die Ausführungszeit eines Zyklus immer bei $O(1)$.

<br>

### Space Complexity (Speicherkomplexität): $O(1)$
Die Speicherkomplexität ist streng **konstant**.
* **Zustandsspeicherung:** Das Skript verwendet nur wenige, skalare Variablen zur Zustandserfassung (insbesondere den Boolean `last_pc_status_phy`).
* **Keine akkumulierenden Datenstrukturen:** Es werden keine Arrays, Listen oder History-Logs im Speicher über die Laufzeit hinweg aufgebaut. 
* **Garbage Collection:** Die temporären Netzwerk-Antworten (das JSON-`status_dict`) sind in ihrer Struktur winzig. Sie werden in jedem Schleifendurchlauf neu deklariert und die alte Instanz wird vom Garbage Collector freigegeben. 
* Der maximale Arbeitsspeicher-Bedarf (RAM) des Python-Prozesses ist somit nach oben gedeckelt und bleibt über Monate hinweg exakt gleich $O(1)$.