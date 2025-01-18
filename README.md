# StoveTx

StoveTx is the transmitter component of an IoT (Internet of Things) temperature monitoring system. It works in conjunction with StoveRx (receiver unit) to provide remote temperature monitoring capabilities.

## Hardware Components
- Adafruit Feather M0 microcontroller board
- LoRa radio module for wireless communication
- Temperature sensors:
  - High-temperature sensor for stove monitoring
  - External sensor for ambient temperature readings
- Battery power system with voltage monitoring

## Main Functions
- Collects temperature readings:
  - Stove temperature measurement
  - Ambient/outside temperature measurement
- Monitors battery status:
  - Battery voltage
  - Battery percentage calculation
- Transmits data package to receiver unit
- Power management for long battery life

## Communication Features
- Uses LoRa radio (915 MHz frequency) for long-range transmission
- Regular data transmission intervals
- Acknowledgment reception from StoveRx
- Low-power operation between transmissions

## Key Features
- High-temperature measurement capability
- Dual temperature sensor support
- Battery-powered operation
- Long-range wireless communication
- Energy-efficient design
- Robust error handling
- Automatic retransmission on failed delivery

This transmitter unit is designed to operate reliably in high-temperature environments while providing consistent temperature monitoring data to the StoveRx receiver unit.