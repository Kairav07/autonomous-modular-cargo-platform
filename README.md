# Autonomous Modular Cargo Platform (AMCP)

AMCP is an ESP32-based autonomous robotic cargo platform designed to transport modular payloads using line-following navigation, obstacle detection, and real-time telemetry.

## Features

- ESP32-based control system
- Autonomous line-following navigation
- IR sensor path detection
- Junction detection
- HC-SR04 obstacle detection
- TB6612FNG motor control
- Real-time serial telemetry
- Local Flask-based Cargo OS dashboard
- Modular cargo platform

## Project Structure

- dashboard/ - Python Flask telemetry dashboard
- firmware/ - ESP32 firmware
- hardware/ - Hardware information
- docs/ - Project documentation
- images/ - Prototype images and diagrams

## Running the Dashboard

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
python3 dashboard/app.py
```

Open http://127.0.0.1:8000 in a browser.

## Technologies

- ESP32
- C++
- Python
- Flask
- PySerial
- HTML
- CSS
- JavaScript

## Status

Prototype under development.

## Author

Kairav Murugesh
