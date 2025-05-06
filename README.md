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

  
