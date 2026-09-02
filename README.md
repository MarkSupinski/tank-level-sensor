# ESP32 Non-Invasive Blackwater Tank Level Sensor (SCAD TM1 Style)

A non-contact, external capacitive liquid level monitoring system for marine/RV blackwater (sewage) holding tanks using an ESP32 micro-controller, custom foil tape sensors, ESPHome, and Home Assistant.

---

## 1. Project Overview & Operating Principle

This project implements a non-invasive, SCAD TM1-style capacitive sensor system for monitoring waste/holding tanks on boats or RVs. 

### Key Features
- **Zero Tank Penetration:** No holes, probes, or mechanical parts inside the sewage tank.
- **Solid-State Reliability:** Immune to fouling, clogging, or tissue buildup.
- **Home Assistant Native:** Integrated over Wi-Fi via ESPHome API.
- **Automated Slosh Filtering:** Integrated moving average filter to stabilize readings underway.

### How It Works
Two parallel conductive foil strips are adhered vertically to the **outside wall** of a non-metallic (polyethylene or fiberglass) holding tank. Together, the foil strips, the plastic wall, and the liquid inside form a variable capacitor:
- **Empty Tank:** Air inside the tank yields a lower dielectric constant ($\epsilon_r pprox 1$).
- **Full Tank:** Water and organic waste behind the foil yield a significantly higher dielectric constant ($\epsilon_r pprox 80$).

As liquid rises inside the tank, total electrical capacitance across the foil strips increases linearly. The ESP32's internal touch-sensing peripheral (`esp32_touch`) directly measures this capacitance without requiring external Analog-to-Digital Converters (ADCs) or voltage dividers.

---

## 2. Hardware Requirements & Specifications

### Primary Components
| Component | Specification / Recommendation | Purpose |
| :--- | :--- | :--- |
| **Microcontroller** | ESP32-WROOM-32 Development Board | Reads capacitive pin (GPIO4) & runs ESPHome firmware |
| **Power Supply** | 12V-to-5V DC-DC Buck Converter (e.g., LM2596 or marine USB module) | Steps down marine house battery bank (12V/24V) to clean 5V USB/VIN |
| **Sensor Tape** | 2-inch wide Copper Foil Tape (conductive adhesive) or HVAC Aluminum Foil Tape | External capacitive sensing electrodes |
| **Wiring** | 20–22 AWG Stranded Marine-Grade Copper Wire | Connections from foil strips to ESP32 board |
| **Heat Shrink / Seals** | Marine liquid electrical tape or adhesive-lined heat shrink | Moisture-proofing solder joints on sensor strips |

### System Specifications
- **Tank Depth:** 30 inches (76.2 cm) nominal depth.
- **Power Input:** 12V DC Marine House Battery Bank (nominal 12.0V – 14.6V range).
- **Measurement Pin:** ESP32 GPIO4 (Internal Touch Channel `T0`).
- **Grounding:** Shared common ground between 12V system, Buck Converter output, and ESP32 GND.

---

## 3. Circuit Diagram & Wiring Schematic

### ASCII Schematic

```text
                  12V MARINE HOUSE BANK (12.0V - 14.6V)
                 +-------------------------------------+
                 | (+12V)                       (GND)  |
                 +---|----------------------------|----+
                     |                            |
           +---------v----------------------------v---------+
           |     12V TO 5V DC-DC BUCK STEP-DOWN CONVERTER   |
           +---------|----------------------------|---------+
                     | (+5V)                      | (GND)
                     |                            |
              +------v----------------------------v------+
              | ESP32-WROOM-32 DEVELOPMENT BOARD         |
              |                                          |
              |  GPIO4 (T0 Touch)                GND     |
              +----|------------------------------|------+
                   |                              |
                   |                              |
  BLACKWATER TANK  |                              |
 +-----------------|------------------------------|-----------------+
 |                 |                              |                 |
 |    |============|---|              |-----------|============|    |
 |    |  FOIL STRIP A  |              |   FOIL STRIP B         |    |
 |    |  (TOUCH SIGNAL)|              |   (COMMON GROUND)      |    |
 |    |                | <-- 1.5" --> |                        |    |
 |    |  30" Vertical  |     GAP      |   30" Vertical         |    |
 |    |================|              |========================|    |
 |                                                                  |
 |            [ Stuck to OUTSIDE of polyethylene wall ]             |
 +------------------------------------------------------------------+
```

### Pin Connectivity Summary
1. **Buck Converter Input:** Connected to Marine House 12V (+12V Fuse Segment and Battery Ground).
2. **Buck Converter Output:** 5V output to **ESP32 VIN** pin; GND output to **ESP32 GND**.
3. **Foil Strip A (Signal Electrode):** Soldered/tapped to wire leading directly to **ESP32 GPIO4**.
4. **Foil Strip B (Ground Shield):** Soldered/tapped to wire leading directly to **ESP32 GND**.

---

## 4. Sensor Installation Guide

1. **Surface Preparation:**
   - Clean the exterior vertical wall of the plastic/fiberglass holding tank using isopropyl alcohol. Ensure no grease or residue remains.
2. **Applying Foil Strips:**
   - Cut two 30-inch lengths of 2-inch wide conductive copper or aluminum foil tape.
   - Stick **Foil Strip A** vertically along the tank height, extending from 1 inch above the bottom floor up to the 30-inch max liquid level line.
   - Stick **Foil Strip B** parallel to Strip A, maintaining a uniform **1.5 to 2.0-inch gap** between the edges of the two strips.
3. **Wiring Connections:**
   - Solder an insulated wire lead to the top of **Foil Strip A** and route to **ESP32 GPIO4**.
   - Solder an insulated wire lead to the top of **Foil Strip B** and route to **ESP32 GND**.
   - Coat solder joints with liquid electrical tape to prevent moisture-induced surface oxidation or stray capacitance bridges.

---

## 5. Software Configuration (`esphome.yaml`)

This project uses [ESPHome](https://esphome.io): a YAML configuration that the `esphome` CLI compiles into a native ESP32 firmware binary. The config is split across two files:

| File | Purpose |
| :--- | :--- |
| `esphome.yaml` | Canonical device configuration: board, Wi-Fi, API, OTA, touch sensor, smoothing, and the calibrated level calculation. |
| `secrets.yaml` | Your real Wi-Fi credentials, fallback-AP credentials, and API encryption key. **Gitignored** — real secrets are never committed. |

### Key configuration points

- **Sensor read:** `esp32_touch` on `GPIO4` (touch pad `T0`). On current ESPHome the touch platform ships as a `binary_sensor` only (the old continuous `sensor: esp32_touch` platform was removed), so we read its **raw value** through the public `get_value()` method inside a `template` sensor lambda (`update_interval: 2s`). The touch pad is exposed as a `binary_sensor` (`blackwater_touch`, hidden) purely as a handle to that raw reading.
- **Slosh filtering:** `sliding_window_moving_average` (window `15`, send every `3`) on the raw reading smooths values while the boat/RV is moving.
- **Level calculation:** a `template` sensor maps raw capacitance → 0–100% using the substitutions `cal_raw_empty` / `cal_raw_full`. It is driven from the raw sensor's `on_value` trigger so the level updates as soon as a filtered reading is published (no 60 s polling lag).
- **Calibration** values are top-level `substitutions` in `esphome.yaml` for a one-line edit — no need to edit the C++ lambda (see §6).
- **Reading direction:** on original ESP32 (v1), a larger capacitance (more liquid) **decreases** the raw touch value. Two-point calibration handles this automatically — just record the true values for empty and full.
- **`device_class`:** ESPHome no longer exposes `device_class: fill`; the level is still presented as a percentage via `unit_of_measurement: %`.
- **Fallback AP:** `wifi.ap` plus a `captive_portal:` so the hotspot is usable for troubleshooting if the main Wi-Fi drops.

> **Note:** the full config was once pasted inline in this README; it now lives **only** in `esphome.yaml` so the docs and repo cannot drift. The old inline version also hardcoded Wi-Fi credentials — the refactor moves those to `secrets.yaml`.

---

## 6. Calibration Procedure

To map raw capacitance values accurately to $0\%	ext{--}100\%$ levels:

1. **Initial Deployment:**
   - Ensure `internal_raw: "false"` in `esphome.yaml` so the raw value is visible in Home Assistant logs or dashboard.
2. **Empty Tank Baseline (`raw_empty`):**
   - Pump out and completely flush the holding tank.
   - Allow liquid to settle. Record the steady-state reading of `Blackwater Tank Capacitance Raw` (e.g., `300.0`).
3. **Full Tank Baseline (`raw_full`):**
   - Fill the tank with fresh water to the 30-inch mark (100% capacity).
   - Record the steady-state reading of `Blackwater Tank Capacitance Raw` (e.g., `750.0`).
4. **Firmware Update:**
   - Update the `cal_raw_empty` and `cal_raw_full` substitutions at the top of `esphome.yaml` with your recorded values.
   - Option: set `internal_raw: "true"` to hide the raw diagnostic sensor from Home Assistant.
   - Re-flash the ESP32 over-the-air (OTA).

---

## 7. Troubleshooting & Maintenance

- **Inverted Values / No Response:** Ensure the ground wire of the buck converter and ESP32 GND share a unified reference with the house battery system.
- **Drift or Thermal Noise:** Capacitance readings fluctuate slightly with temperature. Ensure foil tape is sealed well against humidity and moisture ingress using silicone or conformal coating.
- **Sloshing Sensitivity:** Increase `window_size` in the `sliding_window_moving_average` filter (e.g., from `15` to `30`) if boat movement causes erratic readings while underway.

---

## 8. Developer Workflow — Build, Flash & Calibrate

### Prerequisites

- [ESPHome](https://esphome.io) installed locally (Docker or `pip install esphome`).
- One initial flash over USB, so Over-The-Air (OTA) updates work afterward.
- A filled-in `secrets.yaml` (this file is gitignored).

### 1. Set up your secrets
Edit `secrets.yaml` and fill in your real values:

- `wifi_ssid` / `wifi_password` — your 2.4 GHz Wi-Fi network.
- `ap_ssid` / `ap_password` — the fallback hotspot the device exposes if it can't join the network.
- `api_encryption_key` — generate one with `openssl rand -base64 32`.

### 2. Compile
```bash
esphome compile esphome.yaml
```

### 3. Flash over USB (first time)
```bash
esphome run esphome.yaml
```

### 4. Update over-the-air (after the initial USB flash)
```bash
esphome upload esphome.yaml
```

### 5. Calibrate (see §6)
With `internal_raw: "false"`, note the steady-state raw value when the tank is empty and when full. Enter those into `cal_raw_empty` / `cal_raw_full` at the top of `esphome.yaml`, re-flash, then set `internal_raw: "true"` to hide the raw reading from Home Assistant once calibrated.

