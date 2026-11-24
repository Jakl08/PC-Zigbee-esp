# Hauptdokument: PC Power-Controller (README.md)

## Inhaltsverzeichnis
1. [Projektübersicht und Hardware-Setup](#1-projektübersicht-und-hardware-setup)
2. [Behobene Fehler und Logik-Probleme](#2-behobene-fehler-und-logik-probleme)
3. [Migration auf den Seeed Studio XIAO ESP32C6](#3-migration-auf-den-seeed-studio-xiao-esp32c6)

## See:
* [Hardware und Verkabelung](chapter/hardware-und-verkabelung.md)
* [Software: Python-Skript](chapter/software-python-skript.md)
* [Software: Docker-Umgebung](chapter/software-docker-umgebung.md)
* [Backend: FastAPI Webseite auf Proxmox](chapter/backend-fastapi-webseite-auf-proxmox.md)

<br>

---
---

<br>

## 1. Projektübersicht und Hardware-Setup

Das Projekt ermöglicht die Fernsteuerung und Statusüberwachung eines PCs. Eine webbasierte API (FastAPI) dient als Dashboard, während ein lokaler Controller die physische Schaltung am PC vornimmt und den Status an die Website meldet. 

**Hardware-Verkabelung am Raspberry Pi (3,3V Logik):**
*   **Relais (Low-Level-Trigger):** Verbunden mit GPIO 18. Da es "Low-Level" triggert, muss der Pin beim Start auf `GPIO.HIGH` gesetzt werden, damit das Relais nicht sofort beim Booten anzieht.
*   **PC-Status (Power-LED):** Abgegriffen über einen Spannungsteiler und verbunden mit GPIO 17 (Pull-Down). Ein anliegendes `HIGH`-Signal bedeutet, der PC ist eingeschaltet.

### System-Architektur

```drawio
<mxGraphModel dx="1000" dy="1000" grid="1" gridSize="10" guides="1" tooltips="1" connect="1" arrows="1" fold="1" page="1" pageScale="1" pageWidth="827" pageHeight="1169" math="0" shadow="0" background="#ffffff">
  <root>
    <mxCell id="0" />
    <mxCell id="1" parent="0" />
    <mxCell id="2" value="FastAPI Web-Dashboard&#xa;(Domain aus .env)" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#dae8fc;strokeColor=#6c8ebf;fontStyle=1" vertex="1" parent="1">
      <mxGeometry x="120" y="120" width="200" height="80" as="geometry" />
    </mxCell>
    <mxCell id="3" value="Hardware-Controller&#xa;(Raspberry Pi / ESP32C6)" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#d5e8d4;strokeColor=#82b366;fontStyle=1" vertex="1" parent="1">
      <mxGeometry x="480" y="120" width="200" height="80" as="geometry" />
    </mxCell>
    <mxCell id="4" value="PC-Hardware&#xa;&lt;br&gt;- Power-Button (Relais)&lt;br&gt;- Power-LED (Status-Pin)" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#ffe6cc;strokeColor=#d79b00;" vertex="1" parent="1">
      <mxGeometry x="480" y="280" width="200" height="80" as="geometry" />
    </mxCell>
    <mxCell id="5" value="API Polling (30s) &amp;amp; Sync" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;startArrow=classic;startFill=1;strokeWidth=2;" edge="1" parent="1" source="2" target="3">
      <mxGeometry relative="1" as="geometry" />
    </mxCell>
    <mxCell id="6" value="Relais schalten (LOW-Trigger)" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;" edge="1" parent="1" source="3" target="4">
      <mxGeometry x="-0.5" relative="1" as="geometry">
        <Array as="points"><mxPoint x="530" y="240" /><mxPoint x="530" y="240" /></Array>
      </mxGeometry>
    </mxCell>
    <mxCell id="7" value="Status prüfen (3,3V)" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;dashed=1;startArrow=classic;startFill=1;endArrow=none;endFill=0;" edge="1" parent="1" source="3" target="4">
      <mxGeometry relative="1" as="geometry">
        <Array as="points"><mxPoint x="630" y="240" /><mxPoint x="630" y="240" /></Array>
      </mxGeometry>
    </mxCell>
  </root>
</mxGraphModel>
```

<br><br>

## 2. Behobene Fehler und Logik-Probleme

Während der Entwicklung traten verschiedene logische Hürden auf, die wie folgt gelöst wurden:

### Fehler 1: Der `SyntaxError` bei globalen Variablen
*   **Problem:** Das Deklarieren von `global last_pc_status_phy` innerhalb der `try/while`-Schleifenstruktur führte zu `SyntaxError: name is assigned to before global declaration`, weil die Variable weiter oben im Block bereits verarbeitet wurde.
*   **Lösung:** Verzicht auf das `global`-Schlüsselwort. Stattdessen wird der bekannte Status als Argument an die Funktion übergeben: `def check_hardware_integrity(expected_state):`. Die Schleife ruft die Funktion dann gezielt mit `check_hardware_integrity(last_pc_status_phy)` auf.

### Fehler 2: Relais klickt mehrfach / Fehlalarme
*   **Problem:** Wenn die API ein Anschalten forderte, triggerte das Skript das Relais, setzte die Status-Variable auf "AN" und prüfte kurz danach erneut. Da der PC aber 10-15 Sekunden braucht, bis die Power-LED leuchtet, warf das Skript einen Fehlalarm ("Pin ist LOW, Variable ist True"), setzte alles auf "AUS" zurück und triggerte das Relais im nächsten Durchlauf erneut.
*   **Lösung:** Nach dem Auslösen des Relais wurde eine Verzögerungs-Schleife (bis zu 15 Sekunden) eingebaut, in der das Skript aktiv wartet, bis sich der tatsächliche GPIO-Pin-Status ändert (`if get_pc_status_phy(): break`), bevor der neue Zustand in der Variablen gespeichert wird.

### Fehler 3: Status-Sync auf der Website schlägt fehl
*   **Problem:** Die API forderte nicht das Feld `is_on` für Schaltwünsche, sondern `is_requested`. Zudem gab es im Exception-Block einen Namensfehler (`pc_status_phy` statt dem Funktionsaufruf `get_pc_status_phy()`), wodurch der POST-Request fehlschlug.
*   **Lösung:** Das Skript fragt nun `status_dict.get("is_requested", False)` ab. Im Exception-Block wird der Status frisch ausgelesen, in die Schleifenvariable geschrieben und fehlerfrei an `report_status_to_api(last_pc_status_phy)` übergeben.

<br><br>

## 3. Migration auf den Seeed Studio XIAO ESP32C6

Der Raspberry Pi soll durch den weitaus kompakteren und stromsparenderen **Seeed Studio XIAO ESP32C6** ersetzt werden. 

*   **Hardware-Kompatibilität:** Da der ESP32C6 ebenfalls mit einer **3,3V-Logik** arbeitet, können die drei Pins des Relais (VCC, GND, Signal) und die zwei Pins des Status-Sensors (GND, Signal vom Spannungsteiler) 1:1 vom Raspberry Pi übernommen werden.
*   **Software-Anpassung:** Das bestehende Python-Skript (`RPi.GPIO`) läuft auf dem ESP32 nicht direkt. Es müsste auf MicroPython (`machine.Pin`) umgeschrieben werden.
*   **Die Zigbee-Einschränkung:** Obwohl der ESP32C6 einen Zigbee-Chip besitzt, ist es in MicroPython aktuell nicht ohne extremen Programmieraufwand (C/C++ über ESP-IDF) möglich, den Mikrocontroller als standardisiertes Zigbee-Endgerät in einem bestehenden Zigbee2MQTT-Netzwerk anzulernen.
*   **Der empfohlene Weg (WLAN & ESPHome):** Da der zentrale Pi im Netzwerk ohnehin Home Assistant und Mosquitto bereitstellt, wird empfohlen, den ESP32C6 über WLAN mit **ESPHome** zu flashen. Über eine simple YAML-Konfiguration lassen sich das Relais (Switch) und der Status (Sensor) definieren. Der Controller taucht dann automatisch über das Netzwerk in Home Assistant auf und macht jegliche Python- und Zigbee-Umwege überflüssig. Alternativ kann MicroPython in Kombination mit einer MQTT-Bibliothek über WLAN genutzt werden.