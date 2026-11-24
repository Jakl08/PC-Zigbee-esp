# Hardware und Verkabelung (hardware.md)

## 1. Spezifikationen der Bauteile

### Relais-Modul
*   **Typ:** GTIWUNG 1-Kanal Relais Modul 5V mit Optokoppler-Isolation (Ausführliche technische Daten siehe [Datenblatt](../datasheets/1-Channel_5V_Relais.md)).
*   **Elektrische Daten:** Das Modul arbeitet mit einer 5V DC Betriebsspannung bei einem Ruhestrom von 5mA (max. 190mA Arbeitsstrom). Durch die Optokoppler-Isolation wird der GPIO-Pin des Mikrocontrollers geschont, da der benötigte Auslösestrom lediglich 2 bis 5mA beträgt.
*   **Trigger-Verhalten:** Das Modul wird über den integrierten Jumper als Low-Level-Trigger konfiguriert. Ein `LOW`-Signal (0 bis 1,5V) am GPIO 18 schließt den Stromkreis (Relais zieht an). Der GPIO-Pin muss beim Booten zwingend auf `HIGH` initialisiert werden, um unbeabsichtigte Schaltvorgänge zu verhindern.
*   **Anschluss an PC:** Die Kontakte **COM** (Common) und **NO** (Normally Open) des Relais werden parallel zum physischen Power-Taster des PC-Gehäuses an die Mainboard-Pins angeschlossen.

<br>

### Spannungsteiler (Status-Strom)
Die Power-LED-Pins eines PC-Mainboards liefern im eingeschalteten Zustand typischerweise 5V. Da die GPIO-Pins des Raspberry Pi (und ESP32C6) nur maximal 3,3V vertragen, muss die Spannung über einen Spannungsteiler reduziert werden, um Hardwareschäden zu vermeiden.
*   **Widerstand 1 (R1 - im Signalweg):** 10 kΩ
*   **Widerstand 2 (R2 - gegen Masse):** 20 kΩ
*   **Berechnung:** $V_{out}=V_{in}\times\frac{R_2}{R_1+R_2}\rightarrow5V\times\frac{20k\Omega}{30k\Omega}=3,33V$.

<br><br>

## 2. GPIO-Belegung (Raspberry Pi / ESP32C6)

Das System nutzt eine 3,3V-Logik für alle Steuer- und Signalleitungen. Die folgende Tabelle zeigt die exakte Verkabelung zwischen dem Mikrocontroller (Raspberry Pi oder ESP32C6) und den externen Bauteilen.

| Controller Pin | Typ / Konfiguration | Ziel-Komponente | Ziel-Anschluss | Funktion |
| :--- | :--- | :--- | :--- | :--- |
| **GPIO 18** | Ausgang (3,3V) | Relais-Modul | `IN` | Steuert das Relais für den physischen Power-Taster (Low-Level-Trigger). |
| **GPIO 17** | Eingang (3,3V, Pull-Down) | Spannungsteiler | Mittelabgriff (zw. R1 & R2) | Liest das reduzierte 3,33V-Signal der PC Power-LED aus. |
| **5V / VCC** | Stromversorgung | Relais-Modul | `DC+` | Stellt die Betriebsspannung für das Optokoppler-Relais bereit. |
| **GND** | Masse | Relais-Modul | `DC-` | Gemeinsame Masse für den Relais-Stromkreis. |
| **GND** | Masse | Spannungsteiler | Ende von R2 / PC-GND | Gemeinsame Masse für den PC-Status-Stromkreis. |



<br><br>

## 3. Schaltpläne (draw.io)

### Verkabelung des Relais (Power-Taster)

```drawio
<mxGraphModel dx="1000" dy="1000" grid="1" gridSize="10" guides="1" tooltips="1" connect="1" arrows="1" fold="1" page="1" pageScale="1" pageWidth="827" pageHeight="1169" math="0" shadow="0" background="#ffffff">
  <root>
    <mxCell id="0" />
    <mxCell id="1" parent="0" />
    <!-- Controller -->
    <mxCell id="c_box" value="Raspberry Pi / ESP32&#xa;(Controller)" style="rounded=0;whiteSpace=wrap;html=1;fillColor=#dae8fc;strokeColor=#6c8ebf;verticalAlign=top;paddingTop=10;fontStyle=1" vertex="1" parent="1">
      <mxGeometry x="80" y="120" width="140" height="160" as="geometry" />
    </mxCell>
    <mxCell id="c_5v" value="5V / VCC" style="text;html=1;strokeColor=none;fillColor=none;align=right;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="110" y="160" width="100" height="20" as="geometry" />
    </mxCell>
    <mxCell id="c_gnd" value="GND" style="text;html=1;strokeColor=none;fillColor=none;align=right;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="110" y="200" width="100" height="20" as="geometry" />
    </mxCell>
    <mxCell id="c_gpio" value="GPIO 18" style="text;html=1;strokeColor=none;fillColor=none;align=right;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="110" y="240" width="100" height="20" as="geometry" />
    </mxCell>

    <!-- Relay -->
    <mxCell id="r_box" value="GTIWUNG Relais&#xa;(Low-Level Trigger)" style="rounded=0;whiteSpace=wrap;html=1;fillColor=#d5e8d4;strokeColor=#82b366;verticalAlign=top;paddingTop=10;fontStyle=1" vertex="1" parent="1">
      <mxGeometry x="340" y="120" width="140" height="160" as="geometry" />
    </mxCell>
    <mxCell id="r_dcp" value="DC+" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="350" y="160" width="40" height="20" as="geometry" />
    </mxCell>
    <mxCell id="r_dcn" value="DC-" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="350" y="200" width="40" height="20" as="geometry" />
    </mxCell>
    <mxCell id="r_in" value="IN" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="350" y="240" width="40" height="20" as="geometry" />
    </mxCell>
    <mxCell id="r_com" value="COM" style="text;html=1;strokeColor=none;fillColor=none;align=right;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="430" y="180" width="40" height="20" as="geometry" />
    </mxCell>
    <mxCell id="r_no" value="NO" style="text;html=1;strokeColor=none;fillColor=none;align=right;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="430" y="220" width="40" height="20" as="geometry" />
    </mxCell>

    <!-- PC Mainboard -->
    <mxCell id="pc_box" value="PC Mainboard" style="rounded=0;whiteSpace=wrap;html=1;fillColor=#ffe6cc;strokeColor=#d79b00;verticalAlign=top;paddingTop=10;fontStyle=1" vertex="1" parent="1">
      <mxGeometry x="600" y="120" width="140" height="160" as="geometry" />
    </mxCell>
    <mxCell id="pc_sw1" value="PWR_SW Pin" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="610" y="180" width="100" height="20" as="geometry" />
    </mxCell>
    <mxCell id="pc_sw2" value="PWR_SW Pin" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="610" y="220" width="100" height="20" as="geometry" />
    </mxCell>

    <!-- Wiring Left -->
    <mxCell id="w1" style="endArrow=none;html=1;strokeWidth=2;strokeColor=#ff0000;" edge="1" parent="1">
      <mxGeometry relative="1" as="geometry">
        <mxPoint x="220" y="170" as="sourcePoint" />
        <mxPoint x="340" y="170" as="targetPoint" />
      </mxGeometry>
    </mxCell>
    <mxCell id="w2" style="endArrow=none;html=1;strokeWidth=2;strokeColor=#000000;" edge="1" parent="1">
      <mxGeometry relative="1" as="geometry">
        <mxPoint x="220" y="210" as="sourcePoint" />
        <mxPoint x="340" y="210" as="targetPoint" />
      </mxGeometry>
    </mxCell>
    <mxCell id="w3" style="endArrow=none;html=1;strokeWidth=2;strokeColor=#0000ff;" edge="1" parent="1">
      <mxGeometry relative="1" as="geometry">
        <mxPoint x="220" y="250" as="sourcePoint" />
        <mxPoint x="340" y="250" as="targetPoint" />
      </mxGeometry>
    </mxCell>

    <!-- Wiring Right -->
    <mxCell id="w4" style="endArrow=none;html=1;strokeWidth=2;strokeColor=#000000;" edge="1" parent="1">
      <mxGeometry relative="1" as="geometry">
        <mxPoint x="480" y="190" as="sourcePoint" />
        <mxPoint x="600" y="190" as="targetPoint" />
      </mxGeometry>
    </mxCell>
    <mxCell id="w5" style="endArrow=none;html=1;strokeWidth=2;strokeColor=#000000;" edge="1" parent="1">
      <mxGeometry relative="1" as="geometry">
        <mxPoint x="480" y="230" as="sourcePoint" />
        <mxPoint x="600" y="230" as="targetPoint" />
      </mxGeometry>
    </mxCell>
  </root>
</mxGraphModel>
```

### Verkabelung des Status-Sensors (Spannungsteiler)

```drawio
<mxGraphModel dx="1000" dy="1000" grid="1" gridSize="10" guides="1" tooltips="1" connect="1" arrows="1" fold="1" page="1" pageScale="1" pageWidth="827" pageHeight="1169" math="0" shadow="0" background="#ffffff">
  <root>
    <mxCell id="0" />
    <mxCell id="1" parent="0" />
    
    <!-- PC Mainboard -->
    <mxCell id="pc_box" value="PC Mainboard" style="rounded=0;whiteSpace=wrap;html=1;fillColor=#ffe6cc;strokeColor=#d79b00;verticalAlign=top;paddingTop=10;fontStyle=1" vertex="1" parent="1">
      <mxGeometry x="80" y="120" width="140" height="160" as="geometry" />
    </mxCell>
    <mxCell id="pc_5v" value="PWR_LED+ (5V)" style="text;html=1;strokeColor=none;fillColor=none;align=right;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="110" y="150" width="100" height="20" as="geometry" />
    </mxCell>
    <mxCell id="pc_gnd" value="PWR_LED- (GND)" style="text;html=1;strokeColor=none;fillColor=none;align=right;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="110" y="230" width="100" height="20" as="geometry" />
    </mxCell>
    
    <!-- Controller -->
    <mxCell id="pi_box" value="Raspberry Pi / ESP32" style="rounded=0;whiteSpace=wrap;html=1;fillColor=#dae8fc;strokeColor=#6c8ebf;verticalAlign=top;paddingTop=10;fontStyle=1" vertex="1" parent="1">
      <mxGeometry x="540" y="120" width="140" height="160" as="geometry" />
    </mxCell>
    <mxCell id="pi_gpio" value="GPIO 17 (IN)" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="550" y="150" width="100" height="20" as="geometry" />
    </mxCell>
    <mxCell id="pi_gnd" value="GND" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=middle;" vertex="1" parent="1">
      <mxGeometry x="550" y="230" width="100" height="20" as="geometry" />
    </mxCell>
    
    <!-- Resistors -->
    <mxCell id="r1" value="R1 (10 kΩ)" style="rounded=0;whiteSpace=wrap;html=1;fillColor=#e1d5e7;strokeColor=#9673a6;" vertex="1" parent="1">
      <mxGeometry x="260" y="140" width="80" height="40" as="geometry" />
    </mxCell>
    <mxCell id="r2" value="R2&#xa;(20 kΩ)" style="rounded=0;whiteSpace=wrap;html=1;fillColor=#e1d5e7;strokeColor=#9673a6;" vertex="1" parent="1">
      <mxGeometry x="400" y="170" width="40" height="60" as="geometry" />
    </mxCell>
    
    <!-- Wiring -->
    <!-- 5V from PC to R1 -->
    <mxCell id="w1" style="endArrow=none;html=1;strokeWidth=2;strokeColor=#ff0000;" edge="1" parent="1">
      <mxGeometry relative="1" as="geometry">
        <mxPoint x="220" y="160" as="sourcePoint" />
        <mxPoint x="260" y="160" as="targetPoint" />
      </mxGeometry>
    </mxCell>
    <!-- R1 to Junction and Pi -->
    <mxCell id="w2" style="endArrow=none;html=1;strokeWidth=2;strokeColor=#ff0000;" edge="1" parent="1">
      <mxGeometry relative="1" as="geometry">
        <mxPoint x="340" y="160" as="sourcePoint" />
        <mxPoint x="540" y="160" as="targetPoint" />
      </mxGeometry>
    </mxCell>
    <!-- Junction to R2 -->
    <mxCell id="w3" style="endArrow=none;html=1;strokeWidth=2;strokeColor=#ff0000;" edge="1" parent="1">
      <mxGeometry relative="1" as="geometry">
        <mxPoint x="420" y="160" as="sourcePoint" />
        <mxPoint x="420" y="170" as="targetPoint" />
      </mxGeometry>
    </mxCell>
    <!-- Junction Dot -->
    <mxCell id="dot1" value="" style="ellipse;whiteSpace=wrap;html=1;aspect=fixed;fillColor=#000000;" vertex="1" parent="1">
      <mxGeometry x="416" y="156" width="8" height="8" as="geometry" />
    </mxCell>
    
    <!-- GND Line from PC to Pi -->
    <mxCell id="w4" style="endArrow=none;html=1;strokeWidth=2;strokeColor=#000000;" edge="1" parent="1">
      <mxGeometry relative="1" as="geometry">
        <mxPoint x="220" y="240" as="sourcePoint" />
        <mxPoint x="540" y="240" as="targetPoint" />
      </mxGeometry>
    </mxCell>
    <!-- R2 to GND Line -->
    <mxCell id="w5" style="endArrow=none;html=1;strokeWidth=2;strokeColor=#000000;" edge="1" parent="1">
      <mxGeometry relative="1" as="geometry">
        <mxPoint x="420" y="230" as="sourcePoint" />
        <mxPoint x="420" y="240" as="targetPoint" />
      </mxGeometry>
    </mxCell>
    <!-- GND Dot -->
    <mxCell id="dot2" value="" style="ellipse;whiteSpace=wrap;html=1;aspect=fixed;fillColor=#000000;" vertex="1" parent="1">
      <mxGeometry x="416" y="236" width="8" height="8" as="geometry" />
    </mxCell>
  </root>
</mxGraphModel>
```
