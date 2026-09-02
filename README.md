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

This complete `esphome.yaml` file defines the device configuration, reads the raw touch pin value, smooths signal fluctuations, and calculates percentage fill.

```yaml
esphome:
  name: blackwater-tank-monitor
  comment: "Non-invasive SCAD TM1 Style Capacitive Blackwater Level Sensor"

esp32:
  board: esp32dev
  framework:
    type: arduino

# Network Connectivity
wifi:
  ssid: "YOUR_WIFI_SSID"
  password: "YOUR_WIFI_PASSWORD"

  # Fallback AP in case Wi-Fi connection fails
  ap:
    ssid: "Blackwater-Monitor-Fallback"
    password: "fallbackpassword"

logger:
  level: INFO

# Native Home Assistant Integration API
api:
  encryption:
    key: "GENERATED_ENCRYPTION_KEY"

# Over-The-Air Update Capability
ota:
  - platform: esphome

# Hardware Capacitive Touch Configuration
esp32_touch:
  setup_mode: false

sensor:
  # Raw Capacitance Measurement Sensor
  - platform: esp32_touch
    name: "Blackwater Tank Capacitance Raw"
    id: raw_capacitance
    pin: GPIO4
    threshold: 1000
    update_interval: 2s
    internal: false # Set to 'true' after completing calibration to hide from HA dashboard
    filters:
      # Sliding window moving average filter to smooth liquid movement underway
      - sliding_window_moving_average:
          window_size: 15
          send_every: 3

  # Calibrated Tank Level Percentage Output
  - platform: template
    name: "Blackwater Tank Level"
    id: blackwater_level_percent
    unit_of_measurement: "%"
    icon: "mdi:gauge"
    accuracy_decimals: 1
    state_class: "measurement"
    device_class: "fill"
    lambda: |-
      float raw = id(raw_capacitance).state;

      // CALIBRATION PARAMETERS
      // Replace raw_empty and raw_full with readings gathered during setup
      const float raw_empty = 300.0; // Baseline capacitance reading at 0 inches (Empty)
      const float raw_full  = 750.0; // Baseline capacitance reading at 30 inches (Full)

      if (isnan(raw)) {
        return {};
      }

      // Linear mapping equation
      float pct = ((raw - raw_empty) / (raw_full - raw_empty)) * 100.0;

      // Clamp output between 0.0% and 100.0%
      if (pct < 0.0) return 0.0;
      if (pct > 100.0) return 100.0;
      return pct;

text_sensor:
  - platform: version
    name: "Blackwater Monitor ESPHome Version"
```

---

## 6. Calibration Procedure

To map raw capacitance values accurately to $0\%	ext{--}100\%$ levels:

1. **Initial Deployment:**
   - Flash the ESP32 with `internal: false` set under `raw_capacitance` so raw values are visible in Home Assistant logs or dashboard.
2. **Empty Tank Baseline (`raw_empty`):**
   - Pump out and completely flush the holding tank.
   - Allow liquid to settle. Record the steady-state reading of `Blackwater Tank Capacitance Raw` (e.g., `300.0`).
3. **Full Tank Baseline (`raw_full`):**
   - Fill the tank with fresh water to the 30-inch mark (100% capacity).
   - Record the steady-state reading of `Blackwater Tank Capacitance Raw` (e.g., `750.0`).
4. **Firmware Update:**
   - Update `raw_empty` and `raw_full` in the `lambda` section of the YAML code with your recorded values.
   - Option: Set `internal: true` on `raw_capacitance` to hide the raw diagnostic sensor from Home Assistant.
   - Re-flash the ESP32 over-the-air (OTA).

---

## 7. Troubleshooting & Maintenance

- **Inverted Values / No Response:** Ensure the ground wire of the buck converter and ESP32 GND share a unified reference with the house battery system.
- **Drift or Thermal Noise:** Capacitance readings fluctuate slightly with temperature. Ensure foil tape is sealed well against humidity and moisture ingress using silicone or conformal coating.
- **Sloshing Sensitivity:** Increase `window_size` in the `sliding_window_moving_average` filter (e.g., from `15` to `30`) if boat movement causes erratic readings while underway.
