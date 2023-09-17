# ESP32 Bike Trail Traffic Tracker for integration with Trail Mate

The ESP32 Bike Trail Traffic Tracker is a device designed to monitor and record traffic on bike trails using the ESP32 microcontroller. It continuously scans for Bluetooth Low Energy (BLE) devices, logs their unique MAC addresses, and timestamps each entry. The recorded data is then sent to a database for further analysis. This project is intended for tracking and analyzing bike trail usage.

## Table of Contents

- [Requirements](#requirements)
- [Hardware Setup](#hardware-setup)
- [Software Setup](#software-setup)
- [Usage](#usage)
- [Data Storage](#data-storage)
- [Contributing](#contributing)
- [License](#license)

## Requirements

- [ESP32 Development Board](https://www.espressif.com/en/products/devkits)
- [Arduino IDE](https://www.arduino.cc/en/software)
- Database for storing collected data (Firebase)

## Hardware Setup

1. Connect the ESP32 development board to your computer.

2. Ensure that the necessary libraries are installed in your Arduino IDE:
   - ArduinoBLE (for ESP32 BLE functionality)
   - WiFi (for internet connectivity)
   - NTPClient (for fetching current time)

## Software Setup

1. Clone this repository or download the project files.

2. Open the `bike_trail_tracker.ino` file in the Arduino IDE.

3. Configure your Wi-Fi and firebase credentials:
   - Update the Wi-Fi SSID and password.


4. Compile and upload the sketch to your ESP32 development board. 
    - Important note - you might need to change partition scheme in your IDE in order to fit a bigger binary file e. g. *HUGE APP (3MB no OTA)*

## Usage

1. Power on your ESP32 development board.

2. The device will start scanning for nearby BLE devices and log their MAC addresses with timestamps.

3. The recorded data is automatically sent to your chosen database for storage and analysis.

## Data Storage

The scanned data, including MAC addresses and timestamps, is stored in your selected database. You can access, manage, and analyze the data to gain insights into bike trail traffic patterns.

## Contributing

Contributions to this project are welcome. Feel free to open issues or pull requests for improvements or bug fixes.

