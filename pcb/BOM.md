# PCB Bill of Materials (BOM)

**Board:** Tank Level Sensor — ESP32-WROOM-32E + TI FDC1004, **5 V input** (the 12V→5V buck stays external, as in §3).

Reference designators and a suggested part number are given. Use PCBWay's **PCB Assembly (SMT)** service with this BOM, or order the bare board and hand-solder.

| Ref | Qty | Description | Package | Suggested part / notes |
| :--- | :--- | :--- | :--- | :--- |
| **U1** | 1 | Espressif **ESP32-WROOM-32E** (4 MB flash, Wi-Fi/BT) | 38-pin castellated 18×25.5 mm | `ESP32-WROOM-32E-N4` |
| **U2** | 1 | Texas Instruments **FDC1004DGSR** (4-ch cap-to-digital, I²C) | VSSOP-10 / TSSOP-10 | `FDC1004DGSR` (address 0x50) |
| **U3** | 1 | **AMS1117-3.3** LDO (5V→3.3V, 1 A) | SOT-223 | `AMS1117-3.3` |
| U4 | 0 | *(optional)* Silicon Labs **CP2102N-A02-GQFN28** USB-UART bridge | QFN-28 | only if you want on-board USB programming |
| **J1** | 1 | 5V DC input, 2-pin screw terminal 5.08 mm | through-hole | e.g. Phoenix `1727042` / TE `282834-2` |
| **J2** | 1 | Foil A — `CIN1`, 1-pin screw terminal / JST | through-hole | 2-pin terminal also ok if you run both foils to one connector |
| **J3** | 1 | Foil B — `SHLD`, 1-pin screw terminal / JST | through-hole | — |
| J4 | 0 | *(optional)* USB-C receptacle (16-pin, USB 2.0) | SMD | only with U4 fitted |
| **R1** | 1 | 4.7 kΩ — I²C SDA pull-up to 3.3 V | 0603 | 0603, 1% |
| **R2** | 1 | 4.7 kΩ — I²C SCL pull-up to 3.3 V | 0603 | 0603, 1% |
| **R3** | 1 | 10 kΩ — `EN` pull-up to 3.3 V | 0603 | — |
| **R4** | 1 | 10 kΩ — `GPIO0` pull-up to 3.3 V (boot) | 0603 | — |
| R5 | 0 | *(optional)* 10 kΩ — CP2102 self-reset / VBUS (per datasheet) | 0603 | only with U4 |
| **C1** | 1 | 10 µF — regulator input | 0805 | X5R/X7R, ≥16 V |
| **C2** | 1 | 10 µF — regulator output (3.3 V) | 0805 | X5R/X7R, ≥10 V |
| **C3** | 1 | 100 nF — FDC1004 `VDD` decoupling (put next to pin 8) | 0603 | — |
| **C4** | 1 | 100 nF — ESP32 module decoupling | 0603 | — |
| C5 | 0 | *(optional)* 1 µF — CP2102 VDD decoupling | 0603 | only with U4 |
| **D1** | 0 | *(optional)* TVS diode on 5V input (e.g. `SMBJ5.0A`) | SOD-123 / SMB | recommended for marine transients |
| **F1** | 0 | *(optional)* Polyfuse 500 mA on 5V input | 1206 | — |

> **Foil connectors:** keep the `CIN1` and `SHLD` terminals near the tank wiring. A 2-pin terminal (CIN1 + SHLD) is often the neatest.
>
> **Do NOT add an on-board buck for the first rev.** Keep the 12V→5V buck as a separate module (as in §3). If you later want 12V in on this board, add an `MP1584EN` + 22 µH inductor + SS34 Schottky + feedback divider (10 kΩ / 3.3 kΩ) — but that's a second iteration.
