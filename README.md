# Arduino IoT sensors

Arduino project for measuring temperature and humidity sensors.

## Description

This project is part of system, which measure sensors and send results via Meshtastic.

Wemos d1 mini arduino is measuring temperature and humidity sensors, do some analysis for values, and then send data via UART to Meshtastic node. Meshtastic node will send data to configured channel.

## Getting Started

### Dependencies

* This is PlatformIO project (with Visual Studio Code).
* Related hardware schematics and PCB is in [iot-mesh-schema](https://github.com/korsumaki/iot-mesh-schema) repository.

### Meshtastic Node settings

Setup Meshtastic node with following settings

* Enable Serial module
    - RX pin: D7
    - TX pin: D6
    - Serial baud rate: 57600
    - Serial mode: proto
* 

## Next things to do

### Modules

* debug screen
    - Button B to scroll current page

### Main / Business logic

- boot logic
    - delay before uart traffic
    - check meshtastic node readiness with timer
- sending uart messages

### Current consumption

- Initial consumption
    - +estimate based on datasheets and component values
    - +measure actual consumption
- Use sleep mode for Wemos
- Next changes for MKO-01
