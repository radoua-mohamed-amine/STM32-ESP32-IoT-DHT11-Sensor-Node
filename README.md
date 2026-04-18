# STM32 + ESP32 · UART & MQTT Sensor Node

A two-board embedded IoT system. The **STM32 Blue Pill** acts as UART master — it drives LEDs and an LCD display based on temperature thresholds. The **ESP32** reads a DHT11 sensor, sends data to the STM32 over UART, publishes to a **Mosquitto MQTT broker**, and the data is visualised in a **standalone HTML dashboard** with CSV export and offline analysis.

---
## Project structure

```
├── stm32/
│   └── Core/
│       ├── Inc/  (main.h · uart_comm.h · lcd_i2c.h)
│       └── Src/  (main.c · uart_comm.c · lcd_i2c.c · stm32f1xx_hal_msp.c)
├── esp32/
│   └── main.ino
├── mosquitto/
│   └── mosquitto.conf
├── dashboard/
│   ├── index.html
│   ├── style.css
│   └── script.js
├── docs/
│   ├── system_overview.svg
│   └── wiring_diagram.svg
└── README.md
```

---

## System overview


![Wiring Diagram](/docs/wiring_diagram.jpg)
<svg width="680" height="500" viewBox="0 0 680 500" xmlns="http://www.w3.org/2000/svg">
  <rect width="680" height="500" fill="#0a0e1a"/>

  <!-- STM32 board -->
  <rect x="30" y="30" width="170" height="370" rx="10" fill="#1e1b4b" stroke="#7c3aed" stroke-width="1.5"/>
  <text x="115" y="55" text-anchor="middle" font-family="monospace" font-size="12" font-weight="bold" fill="#a78bfa">STM32 Blue Pill</text>
  <text x="115" y="70" text-anchor="middle" font-family="monospace" font-size="9" fill="#554a8a">no CubeMX changes needed</text>

  <!-- STM32 pins -->
  <rect x="36" y="85"  width="48" height="22" rx="4" fill="#2d2a6e" stroke="#7c3aed" stroke-width="0.5"/>
  <text x="60" y="100" text-anchor="middle" font-family="monospace" font-size="10" fill="#c4b5fd">PA9</text>
  <rect x="36" y="115" width="48" height="22" rx="4" fill="#2d2a6e" stroke="#7c3aed" stroke-width="0.5"/>
  <text x="60" y="130" text-anchor="middle" font-family="monospace" font-size="10" fill="#c4b5fd">PA10</text>
  <rect x="36" y="145" width="48" height="22" rx="4" fill="#3b2800" stroke="#d97706" stroke-width="0.5"/>
  <text x="60" y="160" text-anchor="middle" font-family="monospace" font-size="10" fill="#fbbf24">PB6</text>
  <rect x="36" y="175" width="48" height="22" rx="4" fill="#3b2800" stroke="#d97706" stroke-width="0.5"/>
  <text x="60" y="190" text-anchor="middle" font-family="monospace" font-size="10" fill="#fbbf24">PB7</text>

  <!-- RGB LED pins -->
  <rect x="36" y="215" width="48" height="22" rx="4" fill="#1a0a2e" stroke="#3b82f6" stroke-width="0.5"/>
  <text x="60" y="230" text-anchor="middle" font-family="monospace" font-size="10" fill="#93c5fd">PB1</text>
  <rect x="36" y="245" width="48" height="22" rx="4" fill="#0a1f0a" stroke="#16a34a" stroke-width="0.5"/>
  <text x="60" y="260" text-anchor="middle" font-family="monospace" font-size="10" fill="#4ade80">PB0</text>
  <rect x="36" y="275" width="48" height="22" rx="4" fill="#1c0505" stroke="#ef4444" stroke-width="0.5"/>
  <text x="60" y="290" text-anchor="middle" font-family="monospace" font-size="10" fill="#f87171">PA1</text>

  <rect x="36" y="315" width="48" height="22" rx="4" fill="#1a1a1a" stroke="#444" stroke-width="0.5"/>
  <text x="60" y="330" text-anchor="middle" font-family="monospace" font-size="10" fill="#888">GND</text>
  <rect x="36" y="345" width="48" height="22" rx="4" fill="#1a1a1a" stroke="#444" stroke-width="0.5"/>
  <text x="60" y="360" text-anchor="middle" font-family="monospace" font-size="10" fill="#888">3.3V</text>

  <!-- STM32 connection labels -->
  <line x1="84" y1="96"  x2="210" y2="96"  stroke="#7c3aed" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="214" y="100" font-family="monospace" font-size="9" fill="#a78bfa">TX → ESP32 GPIO16 (RX2)</text>
  <line x1="84" y1="126" x2="210" y2="126" stroke="#7c3aed" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="214" y="130" font-family="monospace" font-size="9" fill="#a78bfa">RX ← ESP32 GPIO17 (TX2)</text>
  <line x1="84" y1="156" x2="210" y2="156" stroke="#d97706" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="214" y="160" font-family="monospace" font-size="9" fill="#fbbf24">SCL → LCD SCL</text>
  <line x1="84" y1="186" x2="210" y2="186" stroke="#d97706" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="214" y="190" font-family="monospace" font-size="9" fill="#fbbf24">SDA → LCD SDA</text>

  <!-- RGB pin labels -->
  <line x1="84" y1="226" x2="210" y2="226" stroke="#3b82f6" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="214" y="230" font-family="monospace" font-size="9" fill="#93c5fd">→ 220Ω → RGB Blue pin</text>
  <line x1="84" y1="256" x2="210" y2="256" stroke="#16a34a" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="214" y="260" font-family="monospace" font-size="9" fill="#4ade80">→ 220Ω → RGB Green pin</text>
  <line x1="84" y1="286" x2="210" y2="286" stroke="#ef4444" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="214" y="290" font-family="monospace" font-size="9" fill="#f87171">→ 220Ω → RGB Red pin</text>

  <line x1="84" y1="326" x2="210" y2="326" stroke="#444" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="214" y="330" font-family="monospace" font-size="9" fill="#888">Common GND (shared with ESP32)</text>
  <line x1="84" y1="356" x2="210" y2="356" stroke="#444" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="214" y="360" font-family="monospace" font-size="9" fill="#888">LCD VCC · 3.3V</text>

  <!-- ESP32 board -->
  <rect x="460" y="30" width="185" height="250" rx="10" fill="#0c1a3a" stroke="#3b82f6" stroke-width="1.5"/>
  <text x="552" y="55" text-anchor="middle" font-family="monospace" font-size="12" font-weight="bold" fill="#60a5fa">ESP32 Dev Module</text>

  <!-- ESP32 pins -->
  <rect x="466" y="72"  width="56" height="22" rx="4" fill="#0a1f4a" stroke="#3b82f6" stroke-width="0.5"/>
  <text x="494" y="87"  text-anchor="middle" font-family="monospace" font-size="10" fill="#93c5fd">GPIO16</text>
  <rect x="466" y="102" width="56" height="22" rx="4" fill="#0a1f4a" stroke="#3b82f6" stroke-width="0.5"/>
  <text x="494" y="117" text-anchor="middle" font-family="monospace" font-size="10" fill="#93c5fd">GPIO17</text>
  <rect x="466" y="132" width="56" height="22" rx="4" fill="#042f2e" stroke="#0d9488" stroke-width="0.5"/>
  <text x="494" y="147" text-anchor="middle" font-family="monospace" font-size="10" fill="#2dd4bf">GPIO4</text>
  <rect x="466" y="162" width="56" height="22" rx="4" fill="#1a1a1a" stroke="#444" stroke-width="0.5"/>
  <text x="494" y="177" text-anchor="middle" font-family="monospace" font-size="10" fill="#888">GND</text>
  <rect x="466" y="192" width="56" height="22" rx="4" fill="#1a1a1a" stroke="#444" stroke-width="0.5"/>
  <text x="494" y="207" text-anchor="middle" font-family="monospace" font-size="10" fill="#888">3.3V</text>
  <rect x="466" y="222" width="56" height="22" rx="4" fill="#1a1a1a" stroke="#444" stroke-width="0.5"/>
  <text x="494" y="237" text-anchor="middle" font-family="monospace" font-size="10" fill="#888">VIN 5V</text>

  <!-- ESP32 labels -->
  <line x1="522" y1="83"  x2="548" y2="83"  stroke="#3b82f6" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="552" y="87"  font-family="monospace" font-size="9" fill="#93c5fd">RX2 ← STM32 PA9</text>
  <line x1="522" y1="113" x2="548" y2="113" stroke="#3b82f6" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="552" y="117" font-family="monospace" font-size="9" fill="#93c5fd">TX2 → STM32 PA10</text>
  <line x1="522" y1="143" x2="548" y2="143" stroke="#0d9488" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="552" y="147" font-family="monospace" font-size="9" fill="#2dd4bf">DHT11 + 1kΩ pullup</text>
  <line x1="522" y1="173" x2="548" y2="173" stroke="#444" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="552" y="177" font-family="monospace" font-size="9" fill="#888">Common GND</text>
  <line x1="522" y1="203" x2="548" y2="203" stroke="#444" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="552" y="207" font-family="monospace" font-size="9" fill="#888">DHT11 VCC</text>
  <line x1="522" y1="233" x2="548" y2="233" stroke="#444" stroke-width="1" stroke-dasharray="4 3"/>
  <text x="552" y="237" font-family="monospace" font-size="9" fill="#888">USB 5V power</text>

  <!-- RGB LED detail box -->
  <rect x="460" y="305" width="195" height="110" rx="8" fill="#111827" stroke="#7c3aed" stroke-width="1"/>
  <text x="557" y="325" text-anchor="middle" font-family="monospace" font-size="11" font-weight="bold" fill="#c4b5fd">RGB LED (common cathode)</text>
  <circle cx="480" cy="345" r="6" fill="#3b82f6"/>
  <text x="492" y="349" font-family="monospace" font-size="9" fill="#93c5fd">Blue  → 220Ω → PB1  (&lt;20°C)</text>
  <circle cx="480" cy="365" r="6" fill="#22c55e"/>
  <text x="492" y="369" font-family="monospace" font-size="9" fill="#4ade80">Green → 220Ω → PB0  (20–30°C)</text>
  <circle cx="480" cy="385" r="6" fill="#ef4444"/>
  <text x="492" y="389" font-family="monospace" font-size="9" fill="#f87171">Red   → 220Ω → PA1  (≥30°C)</text>
  <text x="557" y="408" text-anchor="middle" font-family="monospace" font-size="9" fill="#555">Longest pin (GND) → common GND</text>

  <!-- DHT11 detail box -->
  <rect x="30" y="425" width="250" height="55" rx="8" fill="#042f2e" stroke="#0d9488" stroke-width="1"/>
  <text x="155" y="443" text-anchor="middle" font-family="monospace" font-size="11" font-weight="bold" fill="#2dd4bf">DHT11 (flat face toward you)</text>
  <text x="155" y="459" text-anchor="middle" font-family="monospace" font-size="9" fill="#4a9a90">Pin1→3.3V  Pin2→GPIO4+1kΩ  Pin4→GND</text>
  <text x="155" y="472" text-anchor="middle" font-family="monospace" font-size="9" fill="#3a7a70">1kΩ pullup between Pin2 and 3.3V</text>

  <!-- GND shared note -->
  <rect x="295" y="425" width="350" height="35" rx="8" fill="#111" stroke="#333" stroke-width="0.5"/>
  <text x="470" y="440" text-anchor="middle" font-family="monospace" font-size="10" font-weight="bold" fill="#888">STM32 GND ↔ ESP32 GND — shared rail</text>
  <text x="470" y="454" text-anchor="middle" font-family="monospace" font-size="9" fill="#444">Both 3.3V logic · direct UART · no voltage divider</text>
</svg>


---

## Wiring diagram

![System Overview](https://drive.google.com/file/d/1tRHUJXq54mDhN2BEqbL4ciY0dBa4CfTw/view)

---

## Hardware

### Components

| Component                 | Role                                         |
| ------------------------- | -------------------------------------------- |
| STM32F103C8T6 (Blue Pill) | UART master · LED control · LCD display      |
| ESP32 Dev Module          | UART slave · DHT11 reader · WiFi · MQTT      |
| DHT11                     | Temperature + humidity sensor (on ESP32)     |
| LCD 1602 I2C              | Displays temperature and humidity (on STM32) |
| RGB LED + 3 \* 220Ω       | Temperature indicator                        |

### Pin connections

**STM32 Blue Pill**

| Pin       | Connected to         |
| --------- | -------------------- |
| PA9 (TX)  | ESP32 GPIO16 (RX2)   |
| PA10 (RX) | ESP32 GPIO17 (TX2)   |
| PB6 (SCL) | LCD SCL              |
| PB7 (SDA) | LCD SDA              |
| PB0       | 220Ω → RGB Green pin |
| PB1       | 220Ω → RGB Blue pin  |
| PA1       | 220Ω → RGB Red pin   |
| 3.3V      | LCD VCC              |
| GND       | Common GND rail      |

**ESP32 Dev Module**

| Pin          | Connected to    |
| ------------ | --------------- |
| GPIO16 (RX2) | STM32 PA9 (TX)  |
| GPIO17 (TX2) | STM32 PA10 (RX) |
| GPIO4        | DHT11 data      |
| 3.3V         | DHT11 VCC       |
| GND          | Common GND rail |
| VIN          | 5V USB power    |

---

## Software architecture

### STM32 — STM32CubeIDE + HAL (C)

| File                           | Role                                                    |
| ------------------------------ | ------------------------------------------------------- |
| `Core/Src/main.c`              | Application loop · JSON parser · LED logic · LCD update |
| `Core/Src/uart_comm.c`         | Interrupt-driven UART RX with 128-byte ring buffer      |
| `Core/Src/lcd_i2c.c`           | LCD driver                                              |
| `Core/Src/stm32f1xx_hal_msp.c` | GPIO and clock low-level init for USART1 and I2C1       |

**CubeMX configuration:**

| Peripheral | Setting                                              |
| ---------- | ---------------------------------------------------- |
| USART1     | Async · 115200 baud · 8N1 · Global interrupt enabled |
| I2C1       | Standard mode · 100 kHz                              |
| PB0, PB1   | GPIO Output Push-Pull (Green, Yellow LEDs)           |
| PA1        | GPIO Output Push-Pull (Red LED)                      |
| RCC        | HSE Crystal → PLL × 9 → 72 MHz                       |

### ESP32 — Arduino IDE

**Libraries required:**

| Library                 | Source                            |
| ----------------------- | --------------------------------- |
| DHT sensor library      | Library Manager (Adafruit)        |
| Adafruit Unified Sensor | Library Manager (Adafruit)        |
| PubSubClient            | Library Manager (Nick O'Leary)    |
| ArduinoJson             | Library Manager (Benoit Blanchon) |

**Configure before uploading:**

```cpp
#define WIFI_SSID    "your_wifi_name"
#define WIFI_PASS    "your_wifi_password"
#define MQTT_SERVER  "192.168.1.105"
#define MQTT_PORT    1883
```

---

## UART protocol

```
ESP32  →  STM32 :  {"t":24.5,"h":62.0}\r\n     every 3 seconds
STM32  →  ESP32 :  ACK\r\n
```

---

## MQTT topics

| Topic           | Payload                              | Direction      |
| --------------- | ------------------------------------ | -------------- |
| `stm32/sensors` | `{"temperature":24.5,"humidity":62}` | ESP32 → broker |
| `stm32/status`  | `online` (retained)                  | ESP32 → broker |

---

## LED thresholds

| LED   | Condition                   | Pin |
| ----- | --------------------------- | --- |
| Blue  | Temperature < 20 °C         | PB0 |
| Green | 20 °C ≤ Temperature < 30 °C | PB1 |
| Red   | Temperature ≥ 30 °C         | PA1 |

---

## Mosquitto configuration

`C:\Program Files\mosquitto\mosquitto.conf`

```
listener 1883 0.0.0.0
allow_anonymous true

listener 9001
protocol websockets
allow_anonymous true
```

---

## Startup procedure

```
1. CMD (admin)  →  net start mosquitto
2. Verify       →  netstat -an | findstr "1883 9001"   (both LISTENING)
3. Plug STM32   →  LCD shows "STM32 ready / Wait ESP32..."
4. Plug ESP32   →  wait 10 s → LCD updates → LED turns on
5. Verify MQTT  →  mosquitto_sub -h localhost -t stm32/sensors
6. Browser      →  open dashboard.html in Chrome/Edge → Connect → green dot
7. Export CSV   →  click Export CSV → pick file → appends each session
8. Shutdown     →  Disconnect → unplug boards → net stop mosquitto
```

---

## Dashboard

File: `dashboard/index.html` — open in **Chrome or Edge**.

**Live page:**

- Temperature and humidity cards with min / max
- Animated semicircle gauges
- LED status indicators matching physical LEDs
- 50-point rolling history charts
- Live MQTT message log
- CSV export with append — same file grows across sessions

**Import & Analyse page:**

- Drag and drop any exported CSV
- Statistics: min, max, average, total duration
- Charts from file data with date/time filter
- Paginated colour-coded data table
- Export filtered subset as new CSV




---
