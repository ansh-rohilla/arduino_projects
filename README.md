# Arduino & ESP32 Projects

A collection of embedded systems and IoT projects built with Arduino and ESP32 microcontrollers.

---

## 📁 Repository Structure

```
.
├── Smart_room_env_phone/        # Atmosphere: Smart Room Environment Monitor (ESP32)
│   ├── Smart_room_env_phone.ino # Main Arduino sketch & embedded web server
│   ├── secrets.example.h        # Wi-Fi configuration template
│   └── secrets.h                # Local credentials (git-ignored)
├── sketch_aug5a/                # DHT11 Sensor Test Sketch
│   └── sketch_aug5a.ino
├── .gitignore                   # Ignores secrets, build files & local libraries
└── README.md
```

---

## 🌟 Featured Project: Atmosphere – Smart Room Environment Monitor

An IoT room monitoring station powered by an **ESP32**, **DHT11 sensor**, and **0.96" SSD1306 I2C OLED display**. The ESP32 hosts an interactive web dashboard directly on the local network, allowing real-time monitoring and control from your phone or computer.

### ✨ Key Features
- **Live Environmental Readings**: Periodic sampling of temperature (°C) and relative humidity (%).
- **Onboard OLED Display**: Displays current temperature, humidity, connection status, and status indicators.
- **Embedded Web Dashboard**:
  - Clean, dark-mode responsive UI served directly from ESP32 PROGMEM.
  - Live charts rendered via HTML5 Canvas.
  - Real-time polling via JSON API endpoints (`/data` and `/settings`).
- **Remote Controls via Browser**:
  - Configure temperature warning thresholds.
  - Toggle alert notifications.
  - Toggle OLED screen power and compact display modes remotely.
  - Trigger device reboot from the dashboard.

### 🔌 Pin Configuration

| Component | Pin / Signal | ESP32 GPIO | Notes |
| :--- | :--- | :--- | :--- |
| **DHT11 Sensor** | Data | `GPIO 4` | 10k pull-up resistor (if using bare sensor) |
| **DHT11 Sensor** | VCC / GND | `3.3V` / `GND` | Power supply |
| **SSD1306 OLED** | SDA | `GPIO 21` | I2C Data (Address `0x3C`) |
| **SSD1306 OLED** | SCL | `GPIO 22` | I2C Clock |
| **SSD1306 OLED** | VCC / GND | `3.3V` / `GND` | Power supply |

### 📦 Required Arduino Libraries
Install the following via the Arduino IDE Library Manager:
- **Adafruit SSD1306**
- **Adafruit GFX Library**
- **DHT sensor library** (Adafruit)
- **Adafruit Unified Sensor**

### 🚀 Getting Started

1. **Clone the repository:**
   ```bash
   git clone https://github.com/ansh-rohilla/arduino_projects.git
   cd arduino_projects/Smart_room_env_phone
   ```

2. **Configure Wi-Fi credentials:**
   Copy `secrets.example.h` to `secrets.h`:
   ```bash
   cp secrets.example.h secrets.h
   ```
   Open `secrets.h` and enter your Wi-Fi SSID and Password:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

3. **Upload to ESP32:**
   - Open `Smart_room_env_phone.ino` in Arduino IDE.
   - Select your ESP32 board (e.g., `ESP32 Dev Module`) and port.
   - Click **Upload**.

4. **Access the Dashboard:**
   - Open Serial Monitor at `115200` baud to find the IP address assigned to the board.
   - Open `http://<ESP32_IP_ADDRESS>` in any web browser on the same Wi-Fi network.

---

## 🧪 Project: DHT11 Quick Test (`sketch_aug5a`)

A lightweight starter sketch to verify wiring and functionality of a DHT11 sensor on GPIO 4 with output streamed to the Serial Monitor at 115200 baud.

---

## 🔒 Security Note

Sensitive configuration like network credentials are kept in `secrets.h`, which is listed in `.gitignore` to prevent accidental credential leakage. Never commit `secrets.h` to source control.
