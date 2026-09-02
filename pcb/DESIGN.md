# PCB Design — Tank Level Sensor (KiCad → PCBWay)

This gives you the schematic/netlist, layout rules, and the exact steps to turn this into Gerber files. You draw it in **KiCad** (free) — I can't emit Gerbers directly, but with this document a KiCad run takes ~30 min.

## 1. Architecture

```
 12V marine bank -> [12V->5V buck module, external] -> +5V
                                                        |
   +5V IN (J1) -- AMS1117-3.3 (U3) -- 3.3V rail
      |                                   |
   ESP32-WROOM-32E (U1)  <- 3.3V, GND     |
      | GPIO21 (SDA) --[4k7]-- 3.3V       |
      | GPIO22 (SCL) --[4k7]-- 3.3V       |
      | 3V3 -> U2 VDD                     |
      | GND  -> U2 GND                    |
   FDC1004 (U2)                           |
      | CIN1 (pin2) -> J2 -> Foil A       |
      | SHLD (pin1/6)-> J3 -> Foil B      |
      | (optional U4 USB-UART on GPIO1/3) |
```

## 2. Netlist

| Net | Connections |
| :--- | :--- |
| **+5V** | J1.1, U3 VIN, C1+, (D1/F1 if fitted) |
| **GND** | J1.2, U3 GND, C1−, C2−, U1 GND, U2 GND (pin 7), C3−, C4−, J3 GND, (U4 GND) |
| **+3V3** | U3 OUT, C2+, R1, R2, U1 (3V3), U2 VDD (pin 8), C3+, C4+, R3, R4, (U4 VDD) |
| **I2C_SDA** | U1 GPIO21, R1 (→3V3), U2 SDA (pin 10) |
| **I2C_SCL** | U1 GPIO22, R2 (→3V3), U2 SCL (pin 9) |
| **CIN1** | U2 CIN1 (pin 2), J2.1 (Foil A) |
| **SHLD** | U2 SHLD (pin 1 or 6), J3.1 (Foil B) |
| **EN** | U1 EN, R3 (→3V3), C 0.1 µF to GND, (U4 RTS via transistor for auto-reset) |
| **GPIO0** | U1 GPIO0, R4 (→3V3), (U4 DTR/auto-boot) |
| **(USB-UART, optional)** | U1 GPIO1 (TX)→U4 RXD; U1 GPIO3 (RX)→U4 TXD; U4 D+/D−→J4 USB-C |

## 3. KiCad setup
1. Install **KiCad** (free, macOS) and open a new project.
2. `Preferences → Manage Footprint Libraries` — ensure these are present:
   - `RF_Module:ESP32-WROOM-32` (module footprint)
   - `Sensor:TI_FDC1004` — if not present, make a symbol: VSSOP-10 (pins 1–10 per the pinout table in §11)
   - `Regulator_Linear:AMS1117-3.3` (SOT-223)
   - `Connector_Screw_Terminal:Terminal_1x02_P5.08mm`
   - `Resistor_SMD:R_0603`, `Capacitor_SMD:C_0603`, `C_0805`
   - `Interface_USB:CP2102N` (optional) + `Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12`
3. Place the ESC symbols, wire the nets above, add the footprints.
4. **Critical mind the fill-in:** the FDC1004 symbol has no "standard" KiCad part in some versions — set pin 1 = SHLD, pin 2 = CIN1, 3=CIN2, 4=CIN3, 5=CIN4, 6=SHLD, 7=GND, 8=VDD, 9=SCL, 10=SDA.

## 4. Layout rules (important — this is a capacitive sensor!)
- **Board:** 2 layers, ~40 × 60 mm; 4× 3.2 mm mounting holes; 1.6 mm thickness; **ENIG or HASL lead-free** finish.
- **ESP32 antenna:** the antenna must **overhang the board edge** (or have ≥15 mm keep-out); **no copper or ground pour** under the antenna area. Keep the antenna region clear on both layers.
- **CIN1 (Foil A) is the sensitive net.** Keep the trace short and wide. **Guard it** with a copper track connected to **SHLD** running alongside it (and/or a pour tied to SHLD around the CIN1 pad). This reduces stray/parasitic capacitance — this is the #1 thing that makes the reading stable. **Do not route I²C or 5 V next to CIN1.**
- **Decoupling:** C3 (100 nF) directly at the FDC1004 VDD pin; C1/C2 at the AMS1117 input/output; small caps close to each part's VDD.
- **I²C pull-ups (R1/R2)** close to the FDC1004 or ESP32.
- **Ground:** solid ground pour on the **bottom** layer; add stitching vias. Power net be as continuous as possible.
- **Connectors (J1/J2/J3)** on the same edge for neat wiring; the foil wires should not cross other signals.

## 5. Export Gerbers
1. In KiCad PCB editor: `File → Plot`. Export:
   - Copper top/bottom (`*.gm1`, `*.gtl`), solder mask top/bottom (`*.gts`/`*.gbs`), silkscreen top/bottom (`*.gto`/`*.gbo`), edge cuts (`*.gko`/`*.gm2`).
   - **Drill files:** `PTH/Excellon` and `NPTH/Excellon` (`*.drl` / `*.txt`).
   - Keep the default units mm.
2. Zip the output folder (the `.zip` is the Gerber package).

## 6. Upload to PCBWay
1. Go to **pcbway.com → PCB Instant Quote**.
2. Drag your **Gerber .zip** onto the designer or upload it.
3. Pick parameters:
   - Layers: **2** · Thickness: **1.6 mm** · Finish: **HASL lead-free** (or ENIG) · Color: your choice.
   - Impedance: default. Soldermask: default. Dimensions from the zip.
4. If you want them to **assemble** it too, choose the **PCB Assembly (SMT)** option and upload the BOM — the **`BOM.md`** in this folder lists the parts/refdes (they'll match like-for-like; you can paste the values into the SMT BOM wizard).
5. Order **bare board** only, or **PCB + SMT assembly** (easiest, since the ESP32 and FDC1004 are small SMD parts).

## 7. After assembly
- Flash the ESP32 over USB (or with a USB-UART dongle on the TX/RX/EN/GPIO0 header) and run the §9 smoke test.
- Confirm `FDC1004 found on I2C` and that the raw value moves when Foil A is touched.
