# Pio-Tank-Level-Indicator

*PlatformIO based project for ESP to measure water level*

![Last Commit](https://img.shields.io/github/last-commit/pingmyheart/Pio-Tank-Level-Indicator)
![Repo Size](https://img.shields.io/github/repo-size/pingmyheart/Pio-Tank-Level-Indicator)
![Issues](https://img.shields.io/github/issues/pingmyheart/Pio-Tank-Level-Indicator)
![Pull Requests](https://img.shields.io/github/issues-pr/pingmyheart/Pio-Tank-Level-Indicator)
![License](https://img.shields.io/github/license/pingmyheart/Pio-Tank-Level-Indicator)
![Top Language](https://img.shields.io/github/languages/top/pingmyheart/Pio-Tank-Level-Indicator)
![Language Count](https://img.shields.io/github/languages/count/pingmyheart/Pio-Tank-Level-Indicator)

## 🚀 Overview

Keep your water tanks smart, connected, and worry-free! This project uses the ESP8266 microcontroller and sensors to
measure the water level in your tank, display real-time data on an LCD, and serve it through a web server accessible
from anywhere in your network. Perfect for home automation, smart irrigation, or industrial monitoring setups.

## 🚀 Features

* **Real-time Water Level Measurement**: Accurately detect tank water levels using ultrasonic or float sensors.
* **LCD Display**: Get instant local readings right on your device.
* **Web Server Interface**: Monitor your water tank remotely through any browser on your network.
* **Lightweight & Efficient**: Runs on ESP8266, consuming minimal power.
* **Customizable**: Configure tank size and shape easily through integrated menu

## 🛠️ Components Required

| Component                   | Quantity | Notes                            |
|-----------------------------|----------|----------------------------------|
| ESP8266                     | 1        | Wi-Fi enabled microcontroller    |   
| Ultrasonic Sensor RCWL-1670 | 1        | For water level detection        |   
| LCD1602 I2C Module          | 1        | To use I2C protocol with display |
| LCD1602 Module              | 1        | To show live readings and menù   |
| Jumper Wires                | 1        | For connections                  |
| Power Supply                | 1        | For working                      |

## 🔧 How It Works

* **Sensor Detection**: The water level sensor measures the distance to the water surface.
* **ESP8266 Processing**: The ESP8266 reads sensor data, calculates water level percentage, and updates the LCD.
* **Web Server**: The ESP8266 hosts a simple web page showing real-time water levels with intuitive graphics.
* **Remote Monitoring**: Access the tank status from any device connected to your Wi-Fi network.