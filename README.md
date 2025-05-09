# ESP32 Plant Room Status Monitor

💧🌿 Distributed ESP32-based monitoring system for rooms with plants. Multiple sensor nodes (AHT20 + BMP280 + soil moisture) report data to a central ESP32 hub with OLED display and local web server for real-time graphs and room stats.

## 🧠 Overview

This project uses multiple **ESP32 sensor nodes** connected to:
- **AHT20** (temperature & humidity)
- **BMP280** (pressure)
- **Soil moisture sensors**

Each node sends sensor data over local Wi-Fi to a **central ESP32 hub**, which:

- Displays real-time data on an **80x160 OLED screen**
- Hosts a **local web server** to visualize:
  - Soil moisture levels per plant
  - Environmental conditions across rooms
  - Line charts showing changes over the past 7 days
 
## 🌐 Features

- 📡 Wireless sensor communication (ESP-NOW for energy saving)
- 📊 OLED display for quick information viewing and alerting if something requires attention
- 🌐 Local web UI with historical charts
- 📁 Data logging
- 🛠️ Easy to scale – just add more ESP32 nodes

## ⚙️ Hardware Requirements

- Multiple ESP32 Dev Boards
- AHT20 sensor
- BMP280 sensor
- Soil moisture sensor
- OLED 80x160 SPI display
- Power supply for nodes (power banks in my case)

## 📷 Screenshots, photos, important images

![image](https://github.com/user-attachments/assets/50de6a14-0985-4772-adc3-12b0eb7107a3)

## ⚠️ PROBLEM FOUND: ESP-NOW + Wi-Fi Server (ESP32)

ESP-NOW and Wi-Fi both use the same radio, but they must operate on the **same channel** to coexist. When the ESP32 connects to a router, it joins a **dynamically selected channel**, which can break ESP-NOW communication.

### 🔧 What I did:
- The **hub node connects to Wi-Fi first**, then reads the current Wi-Fi channel and starts ESP-NOW on that channel.
- The **sensor node is hardcoded to use that same channel** (e.g. channel 11).

### 💡 Potential solutions:

1. **Set a fixed Wi-Fi channel** on your router  
   This would guarantee consistent ESP-NOW behavior, but it may **negatively impact your home network**, so I avoided this approach.

2. **Split the ESP-NOW receiver and the HTTP server into two devices**  
   The hub node would focus solely on receiving ESP-NOW data. Then, via **UART or wired serial**, it would forward the data to a second ESP32 (or other device) responsible for hosting the HTTP server.  
   This would eliminate the channel conflict entirely, but I decided not to use more than 1 ESP32 for that purpose.
