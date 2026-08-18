## Introduction

The Wearable Bearable is a prototype wearable haptic timing system that helps groups stay synchronized through shared vibration cues. It is designed for situations such as inclusive orchestras, where some musicians may benefit from an additional tactile signal to reinforce the conductor’s tempo.

The system consists of one master device and one or more slave devices. A phone or other Bluetooth Low Energy (BLE) controller sends a selected tempo in beats per minute (BPM) to the master. The master converts that tempo into timed vibration pulses, activates its own motor, and broadcasts synchronization commands to nearby slaves using ESP-NOW. Each slave reproduces the same short vibration pulses, allowing all connected wearers to receive a coordinated tactile beat without physical connections between devices.

The project explores low-cost wearable haptic feedback as an accessibility tool for shared timing and coordination. The Wearable Bearable is a prototype and is not a medical device; it does not measure heart rate or diagnose, monitor, or treat any medical condition.

## Hardware
The custom KiCad PCB is built around the **Seeed Studio XIAO ESP32-C6** and includes:
- Vibration motor connection and MOSFET driver
- Flyback diode for motor protection
- WS2812B-2020 RGB LED
- Push button
- Supporting resistors and capacitors
The PCB schematic, layout, component footprints, 3D models, and design backups are located in `KiCAD PCB/`.

## Firmware
The Arduino sketches are located in `ESP32c6 Code/`:
- `MasterCode.ino` receives a BPM value over BLE, controls the local vibration motor, and broadcasts the BPM using ESP-NOW.
- `SlaveCode.ino` receives the ESP-NOW command and reproduces the vibration rhythm.
A BPM value of `0` stops the rhythm. Values above `200` BPM are limited to `200`.

## Project Status
Wearable Bearable is currently an engineering prototype. The repository contains the PCB design and proof-of-concept firmware, but it does not yet include a phone application, enclosure, battery/charging design, finalized manufacturing package, or complete assembly instructions.

## Credits
Wearable Bearable was created by me (Akash Saran) and my absolute favorite legends Katherine Xu and Anna Zhou

Special Thanks: Credit to my best friends Katherine Xu for designing the "Bearable" react app (currently on Android) that controls the device and Anna Zhou for maintaining all communication efforts with Wearable's sponsors. 

The repository  includes component symbols, footprints, reference designs, and supporting assets from the **Seeed Studio Open Parts Library and Seeed Studio XIAO ecosystem**. Those third-party materials remain subject to their respective licenses and attribution requirements.

## Status
Project Originally Started: 10/17/2026
Github Saved: 8/18/2026
