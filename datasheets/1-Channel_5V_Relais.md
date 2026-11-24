# Datenblatt: GTIWUNG 1-Kanal Relais Modul 5V

Dieses Dokument fasst die technischen Spezifikationen, Anschlussbelegungen und Konfigurationsmöglichkeiten des im Projekt verwendeten 5V-Relaismoduls zusammen.

## 1. Allgemeine Produktinformationen

*   **Typ:** 1-Kanal Relaisplatine mit Optokoppler-Isolation
*   **Auslöseverhalten:** Flexibel einstellbar (High-Level-Trigger oder Low-Level-Trigger)
*   **Sicherheitsmerkmale:** 
    *   Optokoppler-Isolation für starke Fahrfähigkeit und stabile Leistung.
    *   Fehlertolerantes Design: Bei einer Unterbrechung der Steuerleitung (`IN`) behält das Relais seinen aktuellen Zustand bei und schaltet nicht unkontrolliert.
*   **Statusanzeigen:** 
    *   Betriebsanzeige: Grüne LED (leuchtet bei anliegender Versorgungsspannung)
    *   Relaisstatusanzeige: Rote LED (leuchtet, wenn das Relais angezogen ist)

---

## Technische Spezifikationen

| Eigenschaft | Wert |
| :--- | :--- |
| **Betriebsspannung (Modul)** | 5V DC |
| **Maximale Schaltlast** | AC 250V / 10A<br>DC 30V / 10A |
| **Ruhestrom** | 5mA |
| **Maximaler Arbeitsstrom** | 190mA |
| **Auslösestrom (Trigger)** | 2 - 4mA |
| **Triggerspannung (Low-Level)** | 0 - 1,5V |
| **Triggerspannung (High-Level)**| 2,5 - 5V |
| **Abmessungen (L x B x H)** | 50 mm x 26 mm x 18,5 mm |
| **Befestigungslöcher** | 4 Stück (Ø 3,1 mm), Abstand: 44,5 mm x 20,5 mm |

---

## Pinbelegung und Schnittstellen

### Steuer-Schnittstelle (Eingangsseite)
*   **DC+**: Anschluss für den Pluspol der Stromversorgung (5V).
*   **DC-**: Anschluss für den Minuspol der Stromversorgung (GND).
*   **IN**: Steuerpin zum Triggern des Relais (Empfängt das Signal vom Mikrocontroller).

### Relais-Ausgänge (Lastseite)
*   **NO (Normally Open)**: Schließer-Kontakt. Der Stromkreis ist standardmäßig offen und wird erst geschlossen, wenn das Relais anzieht. *(Dies ist der korrekte Anschluss parallel zum PC-Power-Taster).*
*   **COM (Common)**: Gemeinsamer Mittelkontakt.
*   **NC (Normally Closed)**: Öffner-Kontakt. Der Stromkreis ist standardmäßig geschlossen und wird unterbrochen, wenn das Relais anzieht.

---

## Konfiguration und Zusatzfunktionen

### Trigger-Einstellung (Jumper)
Das Modul wird über einen physischen Jumper an die Logikpegel des jeweiligen Mikrocontrollers angepasst:
*   **Low-Level-Trigger:** Jumper auf `LOW` stecken. Das Relais zieht an, wenn am `IN`-Pin ein niedriges Signal (0 - 1,5V) anliegt.
*   **High-Level-Trigger:** Jumper auf `HIGH` stecken. Das Relais zieht an, wenn am `IN`-Pin ein hohes Signal (2,5 - 5V) anliegt.

### Sicherheits- und Diagnose-Features
*   **Optokoppler-Isolation (SMD):** Trennt den Steuerkreis des Mikrocontrollers galvanisch vom Lastkreis und verhindert zuverlässig Schäden durch Spannungsspitzen oder Rückströme.
*   **Fehlertoleranz:** Wenn die Steuerleitung (`IN`) während des Betriebs unterbrochen wird oder abfällt, schaltet das Relais nicht unkontrolliert, sondern verharrt sicher in seiner Position.
*   **Status-LEDs:** 
    *   **Grün:** Betriebsanzeige (Modul wird mit Strom versorgt).
    *   **Rot:** Relaisstatusanzeige (Leuchtet aktiv auf, wenn das Relais angezogen ist).   