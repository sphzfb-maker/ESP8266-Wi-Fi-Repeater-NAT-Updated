# 🚀 ESP8266 Turbo Wi-Fi Repeater & NAT Router
### *High-Throughput (5 Mbps) & Ultra-Low Latency 160MHz Wi-Fi Range Extender with Modern Web UI*

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP8266-orange.svg)](https://platformio.org/)
[![Clock Speed](https://img.shields.io/badge/CPU-160MHz-green.svg)](https://espressif.com)
[![lwIP](https://img.shields.io/badge/lwIP-NAPT%20v2-purple.svg)](https://savannah.nongnu.org/projects/lwip/)

A high-performance, standalone **Wi-Fi Repeater / WISP-mode NAT Router** firmware written in C++ for the **ESP8266** microcontroller (NodeMCU, Wemos D1 Mini, ESP-12E/F).

It bridges and extends any 2.4 GHz Wi-Fi network without requiring external components, achieving the maximum hardware-theoretical throughput (~5 Mbps) with minimized ping latency and featuring a lightweight, mobile-first Web Management Portal.

---

## ⚡ Key Highlights & Engineering Philosophy

Standard ESP8266 repeaters often suffer from low speeds (< 1 Mbps) and high ping jitter (> 150 ms) due to single-radio half-duplex constraints. This firmware implements targeted low-level optimizations to squeeze the maximum physical performance out of the ESP8266EX silicon:

### 1. Maximum Throughput (~5 Mbps)
* **160 MHz CPU Overclock:** The Xtensa LX106 core is clocked at 160 MHz (double the default 80 MHz), accelerating TCP/UDP translation and packet retransmission in real-time.
* **High-Bandwidth lwIP Buffers:** Compiled with `PIO_FRAMEWORK_ARDUINO_LWIP2_HIGHER_BANDWIDTH`, allocating optimal TCP Window and MSS buffers for maximum download speeds.
* **Maximum RF Output (20.5 dBm):** Tuned for maximal transmission range and signal integrity.

### 2. Ultra-Low Latency & Ping Jitter Reduction
* **Radio Channel Locking (Zero Hopping):** The ESP8266 contains only one physical RF synthesizer. If the SoftAP and the Station (router connection) operate on different channels, the chip is forced into time-sliced channel switching, causing severe packet loss and +100 ms ping spikes. **This firmware automatically detects the upstream router's channel and locks the repeated SoftAP to the exact same physical channel**, eliminating channel-hopping latency entirely.
* **Zero-Sleep RF (`NONE_SLEEP_T`):** Modem sleep is disabled at the SDK register level (`wifi_set_sleep_type(NONE_SLEEP_T)` + `WIFI_NONE_SLEEP`), eliminating beacon sleep wake-up delays.
* **Zero-Delay Dispatch Loop:** Non-blocking event architecture with immediate `yield()` packet processing.

### 3. Lightweight, Reactive Web Portal
* **Live Wi-Fi Scanner (AJAX):** Detects nearby 2.4 GHz networks in real-time with signal strength bars (dBm and %) and security encryption status (🔒/🔓).
* **Live Client List:** Shows the MAC address and IP of every device currently connected to the repeated network, auto-refreshing every 5s.
* **MAC Address Clone:** Optionally override the upstream (WAN) MAC address — useful when an ISP or router locks access to one device's MAC.
* **Automatic Captive Portal:** Automatically opens the configuration page when connecting from Android, iOS, macOS, or Windows devices.
* **Password Visibility Toggle (👁️):** Easily inspect typed passwords to prevent setup errors.
* **Real-Time Live Dashboard:** Displays WAN IP, current MAC, RSSI signal %, number of connected client devices, uptime, and free RAM.
* **Persistent EEPROM Storage:** Stores credentials and MAC override securely with checksum verification across reboots.

### 4. Connection Stability
* **Auto-Reconnect:** `WiFi.setAutoReconnect(true)` plus a periodic forced `WiFi.reconnect()` if the upstream link drops, so the bridge recovers on its own after a router reboot or brief outage.
* **`WiFi.persistent(false)`:** Avoids unnecessary SDK flash writes on every connection cycle.

---

## 🛠️ Hardware Requirements

* Any **ESP8266** board with at least 1MB Flash:
  * NodeMCU v2 / v3 (ESP-12E / ESP-12F)
  * Wemos D1 Mini
  * Generic ESP-01 / ESP-01S (1MB+)
* Micro-USB / USB-C cable for power and flashing (no external sensors or relays required).

---

## 📦 Quick Installation

This repo ships as source only (no precompiled `.bin`) so you always flash a build that matches the code. You'll need [PlatformIO Core](https://platformio.org/install/cli) installed once:
```bash
pip install platformio
```

### Method 1: One-Command Build & Flash (Recommended)

#### Linux / macOS:
```bash
./flash.sh /dev/ttyUSB0
```

#### Windows:
```cmd
flash.bat COM3
```

Both scripts run `pio run` to compile, then flash the result straight from `.pio/build/esp12e/firmware.bin` — no manual steps in between.

---

### Method 2: Manual PlatformIO Commands

1. Build and flash the project:
   ```bash
   pio run -t upload
   ```
2. Monitor serial output (115200 baud):
   ```bash
   pio device monitor
   ```

---

### Method 3: No PC? Build in the Cloud (GitHub Actions)

This repo includes `.github/workflows/build.yml`, which compiles `firmware.bin` automatically on GitHub's servers — no computer or PlatformIO install needed on your side, works entirely from a phone browser:

1. Create a free [GitHub](https://github.com) account if you don't have one.
2. Create a new repository and upload this whole folder (GitHub's "Add file → Upload files" works fine from mobile).
3. Open the **Actions** tab → the "Build ESP8266 Firmware" workflow runs automatically after the upload.
4. Once it finishes (green check), open that run → scroll to **Artifacts** → download `firmware-bin.zip`.
5. Unzip it (most phone file managers can) to get `firmware.bin`, then flash it with any ESP8266 USB flashing app using: address `0x0`, flash mode `dio`, flash frequency `80MHz`, flash size `4MB` (standard for NodeMCU v2/v3 — double check your specific board).

---

## 📖 Step-by-Step User Guide

1. **Power Up & Connect:**
   * Plug the ESP8266 into any 5V USB charger or computer port.
   * On your phone or laptop, look for the open Wi-Fi network: **`ESP8266-Repeater-Setup`**.
2. **Access Web Configuration:**
   * The setup window will pop up automatically. If not, open your browser and navigate to:
   * 👉 **`http://192.168.4.1`**
3. **Configure Networks:**
   * Click **"Scan Networks"** to scan for available Wi-Fi networks.
   * Select your home router's SSID from the dropdown and type your Wi-Fi password.
   * (Optional) Tick **"Clone / Set Custom MAC Address"** to override the MAC used upstream — useful if your ISP or router restricts access by device MAC.
   * Enter the desired name for your new repeated network (e.g. `MyNetwork_EXT`) and a password (minimum 8 characters, or leave blank for open).
4. **Save & Enjoy:**
   * Click **"Save & Connect"**.
   * The ESP8266 will save the configuration to flash, restart, lock to the optimal radio channel, and begin repeating high-speed internet!
5. **Dashboard Management:**
   * Revisit `http://192.168.4.1` at any time while connected to the repeater to inspect live connection metrics, the list of connected client devices (MAC + IP), or reset configuration.

---

## 📂 Project Structure

```
esp8266-wifi-repeater/
├── firmware/
│   └── NOTE.txt                       # Why there's no prebuilt .bin + how to build one
├── src/
│   ├── main.cpp                      # Core L3 NAPT engine, MAC clone & state machine
│   ├── config_storage.h              # EEPROM persistent storage & checksum
│   └── web_portal.h                  # Lightweight dark-mode Web Portal & Captive Portal
├── platformio.ini                    # 160MHz compiler flags & lwIP configuration
├── flash.sh                          # One-click Linux/macOS build + flash script
├── flash.bat                         # One-click Windows build + flash script
├── LICENSE                           # MIT License
└── README.md                         # Documentation & Installation Guide
```

---

## 📜 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details. Free for personal, commercial, and educational use.
