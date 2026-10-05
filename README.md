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
# PicoDeck — Hardware, Pinout, and PicoMesh

## 1. Hardware Platform

The heart of PicoDeck is the **Raspberry Pi Pico 2 W**. The microcontroller serves simultaneously as the control computer, user interface controller, and platform for communication modules and additional features.

The design utilizes the Pico's available interfaces with maximum flexibility, ensuring that individual modules can operate independently without limiting the system's future development.

### PicoDeck Main Pinout

| Function                | GPIO | Interface        |
| ----------------------- | ---: | ---------------- |
| **TFT ST7796**          | | SPI0             |
| SCLK                    | GP10 | SPI0             |
| MOSI                    | GP11 | SPI0             |
| CS                      | GP6 | |
| DC                      | GP8 | |
| BL                      | GP7 | |
| MISO                    | — | Unused           |
| **LoRa SX1262**         | | SPI1             |
| SCK                     | GP18 | SPI1             |
| MOSI                    | GP19 | SPI1             |
| MISO                    | GP16 | SPI1             |
| CS                      | GP26 | |
| DIO1                    | GP20 | |
| BUSY                    | GP21 | |
| RESET                   | GP22 | |
| **GPS**                 | GP17 | UART / SerialPIO |
| GPS PPS                 | GP8* | |
| **Audio MAX98357A**     | | I²S              |
| BCLK                    | GP1 | |
| WS/LRCLK                | GP2 | |
| DOUT   ​​                 | GP3 | |
| **MCP23X17 Keypad**     | | I²C              |
| SDA                     | GP4 | |
| SCL                     | GP5 | |
| V - Batt                | GP28| | Mesuring Battery Voltage
| Free                    | GP0 | 
* The PPS pin was used in one of the configuration versions; The final GPS pin assignment may still depend on the specific board version.

### Keyboard

The keyboard is connected via an **MCP23X17** expander.

The chip supports a matrix of:

**8 × 7 = 56 keys**

MCP23X17 usage:

* GPA0–GPA6 — columns,
* GPA7 — SHIFT LED,
* GPB8–GPB15 — rows.

This allows for a full-sized physical interface without consuming a large number of the microcontroller's GPIO pins.

---

# 2. PicoMesh

**PicoMesh** is a PicoDeck communication module that utilizes LoRa radio.

Its goal is to create a **local network of devices** capable of operating without cellular infrastructure, the Internet, or Wi-Fi.

Any PicoDeck equipped with a LoRa module can act as both a network user and a network node.

## Messages

The primary function of PicoMesh is the exchange of short text messages.

A message includes, among other things:

* sender,
* recipient,
* message ID,
* TTL,
* content.

Example format:

```text
MSG|src|dst|mid|ttl|text
```

The system stores a history of received and sent messages and allows them to be re-sent.

## Multi-node network

PicoMesh is not limited to direct communication between two devices.

A message can be relayed through successive nodes. The **TTL** mechanism limits the maximum number of relays and prevents infinite packet circulation within the network. This allows devices to form a simple **mesh** network, extending communication range beyond the limits of a single radio link.

## Acknowledgments and Retransmissions

PicoMesh was designed with the understanding that LoRa communication can be unreliable.

The system employs:

*   acknowledgments,
*   message identification,
*   a transmission queue,
*   retransmissions,
*   delays between attempts,
*   transmission retries during subsequent communication opportunities.

A key design principle is the separation of **message transmission** from **delivery acknowledgment**.

Users can also manually retry sending a message. Retransmitted messages receive a new identifier, ensuring they are not treated as duplicates of previously sent packets.

## Nodes and Environmental Information

PicoMesh gathers information about other devices within the network.

For any given node, the following information can be displayed:

*   name/identifier,
*   RSSI,
*   hop count,
*   GPS position,
*   distance,
*   direction to the node.

The combination of **LoRa and GPS** allows PicoDeck to function not only as a communication device but also as a navigation aid in the field.

## Beacon

The network also utilizes periodic **BEA** (beacon) messages.

Beacons allow devices to announce their presence to the network and provide the information needed to maintain an up-to-date map of active nodes.

## Radio Parameters

PicoMesh utilizes modules based on the **SX1262/SX1268** chip family. The current test configuration employs, among others:

* bandwidth: **250 kHz**,
* spreading factor: **SF7**,
* coding rate: **4/5**,
* CRC,
* packet synchronization,
* maximum TTL: **5**.
* transmision not coded (not jet)

The parameters can be adjusted as needed—the trade-off primarily involves...
##  Battery Power

PicoDeck is powered by a **Li-Po battery** up to 4000mAh. The battery charging and protection circuitry is based on the **TP4056** module, and the voltage is subsequently regulated by a **3.3 V converter TPS6308, ensuring a proper and stable power supply for the Pico 2 W and the system's other components. This design allows the device to operate as a fully portable, standalone computer.
