# 🎛️ Smart Desk Hub - Firmware

This repository contains the C++ based firmware for the Smart Desk Hub. Running on an ESP8266 microcontroller, this code is responsible for the low-level control of hardware components (sensors, display, buttons), monitoring the desktop environment, and handling network communication with the local backend server.

## 🎯 The Motivation (Why I built this)

This project was designed to solve two specific needs in my daily workspace:

* **Filament Care for 3D Printing:** Since I regularly print physical objects using PLA and PETG, monitoring the room's ambient humidity is crucial. High moisture levels can quickly degrade these filaments, so I needed a reliable, real-time environmental display right on my desk to ensure optimal storage conditions.
* **Mood-Based Lighting Control:** I wanted a physical, tactile interface to instantly adapt my desk's LED lighting to my current activity (Working, Studying, or Relaxing) without interrupting my workflow to open a smart home app.

## 🛠 Features

* **Sensor Data Acquisition:** Continuous reading of ambient temperature and humidity (at 5-second intervals, using a non-blocking approach).
* **Display Control:** Real-time rendering of environmental data (temperature, humidity) and the currently active profile on the OLED monitor.
* **State Management:** Hardware-level polling of 3 dedicated physical push buttons for instant mode switching (*Working, Studying, Relaxing*).
* **Network Communication:** Dispatching HTTP POST requests to the local backend based on the selected lighting and system profile.

## ⚙️ Hardware Requirements

* **Microcontroller:** ESP8266 (e.g., NodeMCU V3 or Wemos D1 Mini)
* **Sensor:** DHT22 Temperature & Humidity Sensor
* **Display:** 128x32 I2C OLED (SSD1306, Address `0x3C`)
* **Actuators:** 3x Push Buttons (handled via software using internal pull-up resistors)

## 🔌 Pinout Configuration

| Component | ESP8266 Pin | Note |
| :--- | :--- | :--- |
| **DHT22 Sensor Data** | `D5` | - |
| **Button 1 (Working)** | `D6` | Configured with `INPUT_PULLUP` |
| **Button 2 (Studying)**| `D7` | Configured with `INPUT_PULLUP` |
| **Button 3 (Relaxing)**| `D3` | Defined as `BTN_GAME` in code |
| **OLED SDA** | `D2` | I2C Default |
| **OLED SCL** | `D1` | I2C Default |

## 💻 Development Environment & Build

This project was built in a standard C++ (Arduino framework) environment. For development and compiling, using **PlatformIO** (VS Code / CLion) or the **Arduino IDE** is recommended after installing the ESP8266 board package.

### Dependencies & Libraries

To successfully compile the firmware, you will need the following libraries:
* `ESP8266WiFi` & `ESP8266HTTPClient` *(Built-in ESP8266 packages)*
* `Wire` *(For I2C communication)*
* `Adafruit GFX Library`
* `Adafruit SSD1306`
* `DHT sensor library` (by Adafruit)
