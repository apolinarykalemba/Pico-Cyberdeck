# Pico-Cyberdeck
RaspberyPico2W computer with keyboard, LoRa, Wifi, 3.5 inch touch screen, GPS, SD card. LoRa, sound. 
# PicoDeck

**PicoDeck** is a portable, multifunctional "cyberdeck" computer built around the **Raspberry Pi Pico 2 W** microcontroller.

The project combines communication, navigation, information, and entertainment into a single, standalone device equipped with its own screen, keyboard, power supply, and communication modules.

## Concept

PicoDeck is designed to be a **standalone field device**—one that does not require a phone or a computer for basic operation.

A single device can perform various functions depending on the selected operating mode:

* **PicoMesh** – local LoRa radio communication and message exchange between devices,
* **PicoNavi / PicoMap** – GPS, positioning, heading, distance, and maps,
* **PicoRadio** – internet radio streaming via Wi-Fi,
* **PicoGame** – simple entertainment features,
* and additional modules to be developed in the future.

All functions utilize a shared user interface and the same hardware platform.

## Hardware

The system is based on the **Raspberry Pi Pico 2 W**, featuring components such as:

* a 480×320 color TFT screen,
* a physical keyboard,
* an SD card for data storage,
* a LoRa module,
* a GPS receiver,
* Wi-Fi connectivity,
* an audio system and speaker,
* a rechargeable battery power supply.

The modular design allows for the addition of new features without the need to build the entire device from scratch.

## Interface

PicoDeck is not designed to be operated via a phone. **The screen and physical keyboard serve as the primary user interface.**

The system features a unified menu for navigating between individual functions. This integrates communication, GPS, radio, and mapping capabilities into a single device rather than treating them as separate projects.

## Communication

A key component of PicoDeck is **PicoMesh**—a proprietary communication network based on LoRa technology.

It enables:

*   sending and receiving messages,
*   device-to-device communication without internet infrastructure,
*   message relaying across multiple nodes,
*   identification of other devices,
*   use of GPS data to determine distance and direction.

This allows PicoDeck to operate even in areas lacking cellular or Wi-Fi coverage.

## A Phased Development Approach

PicoDeck is not being created as a closed device with fixed, predetermined functionality.

The system is being developed **modularly**. New features can be added to the shared hardware and software platform.

A core principle is maintaining the independence of individual functions while ensuring the system runs smoothly. Tasks requiring consistent response times—such as radio communication or audio playback—are decoupled from user interface operations.

## Project Goal

The goal of PicoDeck is to create a **compact, standalone field computer** that combines the following into a single device:

**communication + navigation + information + multimedia + utility tools.**

It is not an attempt to replace a smartphone and all its functions. PicoDeck is intended to be something different:

> **a dedicated physical computer designed precisely for the specific use cases required by its user.**

The entire system is being created as an open project developed step by step—from the electronics and enclosure to the software and subsequent functional modules.
