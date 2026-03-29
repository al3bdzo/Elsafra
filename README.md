# 🌞 Elsafra: Smart Dual-Axis Solar Tracker & Monitor

An ESP32-powered solar tracking system that maximizes energy efficiency by aligning solar panels with the strongest light source using LDR sensors. It features a real-time web dashboard with manual override capabilities, environmental monitoring, and live data visualization.

## 🚀 Features

* **Dual Tracking Modes:**
    * **Auto-Mode:** Uses a weighted differential algorithm to follow light sources dynamically.
    * **Manual-Mode:** Precise servo control via a web-based slider.
* **Environmental Monitoring:** Real-time Temperature and Humidity data via DHT11.
* **Live Analytics:** 5-minute rolling voltage chart and visual gauges.
* **RESTful API:** Provides JSON endpoints for status and remote control.
* **Standalone AP:** Operates as its own Wi-Fi Access Point (`SolarLinkPanel`).

## 🛠️ Hardware Requirements

* **Microcontroller:** ESP32
* **Sensors:** * 2x LDR (Light Dependent Resistors)
    * 1x DHT11 (Temperature & Humidity)
    * Voltage Divider circuit (for ADC 36)
* **Actuators:** 1x Servo Motor (SG90 or MG995)

### Pin Mapping

| Component | ESP32 Pin | Function |
| :--- | :--- | :--- |
| **LDR 1** | GPIO 32 | Left/Top Light Sensor |
| **LDR 2** | GPIO 33 | Right/Bottom Light Sensor |
| **Servo** | GPIO 18 | Panel Rotation |
| **DHT11** | GPIO 4 | Temp/Hum Sensor |
| **Voltage** | GPIO 36 | Analog Voltage Input |

---

## 🏗️ Technical Architecture

### 🛠️ Backend (ESP32 / C++)
The ESP32 acts as both the **Control Unit** and the **Web Server**. It follows a non-blocking asynchronous pattern to balance real-time tracking with network requests.

* **Sensor Fusion:** Implements a weighted differential algorithm. Since LDRs often have manufacturing variances, the code applies a `weight` and `ldrOffset` to normalize readings.
* **Proportional Control:** Instead of binary "Left/Right" movements, it calculates an `adjustment` based on the intensity of the light difference:
    $$adjustment = (LDR_1 \cdot weight - LDR_2 - offset) \cdot K_p$$
* **REST API:** Exposes endpoints that return raw JSON, allowing any frontend client to interface with the hardware.

### 🎨 Frontend (Web Dashboard)
A responsive single-page application (SPA) built with vanilla HTML5, CSS3, and JavaScript.

* **Real-time Visualization:** Uses **Chart.js** to graph voltage fluctuations over a rolling 300-point window.
* **Race-Condition Protection:** Implements **Input Locking**. When a user interacts with the manual slider, background data-syncing is temporarily paused to prevent the UI from "snapping back" to old values.
* **Mobile-Ready:** Optimized with touch-event listeners (`touchstart`/`touchend`) for seamless control on smartphones.

---

## 💻 Software API

| Endpoint | Method | Description |
| :--- | :--- | :--- |
| `/status` | `GET` | Returns JSON with sensor data, servo angle, and mode. |
| `/control` | `GET` | Parameters: `manual=1/0` and `angle=0-180`. |

**Example JSON Response:**
```json
{
  "temperature": 28.5,
  "humidity": 45.0,
  "voltage": 4.12,
  "servo": 95,
  "ldr1": 1800,
  "ldr2": 1650,
  "manual": 0
}
```

---

## 🔧 Installation & Setup

1.  **Arduino IDE Setup:**
    * Install the `ESP32Servo` and `DHT sensor library` (by Adafruit).
2.  **Calibration:**
    * Adjust `ldrOffset` and `Kp` in the "STATE" section of the ESP32 code to calibrate for your specific LDR sensitivity and lighting environment.
3.  **Deployment:**
    * Flash the ESP32 code.
    * Connect your PC/Phone to the Wi-Fi network **SolarLinkPanel** (Password: `12345678`).
4.  **Accessing the Dashboard:**
    * Open the `index.html` file in your browser. It will automatically connect to `http://192.168.4.1`.
