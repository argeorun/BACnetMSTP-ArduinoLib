# BACnetMSTP — Arduino Library

An Arduino library that brings **BACnet MS/TP** (RS-485 fieldbus) connectivity to Arduino-compatible boards.
It wraps the open-source [bacnet-stack](https://github.com/bacnet-stack/bacnet-stack) and exposes a clean C++ API via `BACnetMSTP.h`.

---

## Primary Reference

This library was developed using
**[https://github.com/argeorun/BACnet-MSTP-Arduino](https://github.com/argeorun/BACnet-MSTP-Arduino)**
as the primary reference implementation.

That project contains the full standalone Arduino sketch (`BACnetMSTP-Arduino-R1.ino`) with board-specific platform code, MS/TP datalink adaptation, and BACnet object definitions that this library packages into the Arduino Library Manager format.

---

## Based On

This library is built on the
**[bacnet-stack](https://github.com/bacnet-stack/bacnet-stack)** open-source library
originally written by **Steve Karg** and contributors.

The upstream library provides the complete BACnet protocol stack (encoding/decoding, MS/TP state machine, service handlers). This library adapts the AVR port of that stack for the Arduino IDE build system and multiple Arduino hardware platforms.

See [`src/bacnet/`](src/bacnet/) for the upstream library files (unmodified, all credit to the original authors).
See [`LICENSE`](LICENSE) for the full upstream license notice.

---

## Features

- **BACnet MS/TP** over RS-485 (9600-1,152,000 baud, configurable MAC address)
- **Analog Value objects** (AV x12): ADC inputs (AV:0-3), device config (AV:92-95, writable), diagnostics (AV:96-99)
- **Binary Value objects** (BV x6): Digital outputs D3-D7 (BV:0-4) + LED (BV:99)
- **ReadProperty** and **WriteProperty** service support
- **Who-Is / I-Am** discovery support
- **EEPROM persistence**: device instance, MAC address, baud rate
- **Runtime diagnostics**: uptime hours (AV:99), C-stack headroom (AV:98)
- Verified with **YABE BACnet Explorer** over RS-485

---

## Supported Boards

| Board                              | RS-485 UART | TX pin | RX pin | DE/RE pin |
|------------------------------------|-------------|--------|--------|-----------|
| Arduino Uno / Nano                 | `Serial`    | D1     | D0     | D2        |
| Arduino Mega 2560                  | `Serial1`   | D18    | D19    | D2        |
| STM32 F103C8 (Blue Pill)           | USART3      | PB10   | PB11   | PA12      |
| STM32 Nucleo-144 (F756ZG / F746ZG) | USART6      | PG14   | PG9    | PA12      |
| ESP32                              | `Serial1`   | GPIO25 | GPIO26 | GPIO18    |
| ESP32-S3                           | `Serial2`   | GPIO17 | GPIO16 | GPIO2     |

> Any RS-485 transceiver compatible with your board's logic level works (e.g. MAX485, SN75176, SP3485).

---

## Installation

### Arduino Library Manager (recommended)
1. Open Arduino IDE -> **Sketch -> Include Library -> Manage Libraries...**
2. Search for `BACnetMSTP`
3. Click **Install**

### Manual install
1. Download or clone this repository
2. Copy the folder to `Documents/Arduino/libraries/BACnetMSTP-ArduinoLib/`
3. Restart Arduino IDE

---

## Quick Start

Open **File -> Examples -> BACnetMSTP -> BACnetBasicServer** and follow the comments in the sketch.

1. **Wire** your RS-485 transceiver to the TX, RX and DE/RE pins (see table above).
2. **Select your board** in the Arduino IDE — the sketch auto-configures the correct serial port and pin.
   Or uncomment the matching `OPTION A-F` block manually at the top of the sketch.
3. **Upload** — on first boot factory defaults are written to EEPROM (MAC=10, baud=38400, device instance=260001).
4. **Discover** the device with a BACnet client (YABE, NiagaraAX, etc.) on the same RS-485 network.

See [`examples/BACnetBasicServer/README.md`](examples/BACnetBasicServer/README.md) for the full wiring diagram, object reference table, and CLI build commands.

---

## API Overview

```cpp
#include <BACnetMSTP.h>

// Construct with serial port and DE/RE pin
BACnetMSTP bacnet(Serial1, 2);

void setup() {
    bacnet.begin();          // initialises stack, EEPROM, RS-485
}

void loop() {
    bacnet.task();           // drives MS/TP token-passing -- call as often as possible

    // Read an analog value (e.g. AV:0 = ADC0 in mV)
    float mv = bacnet.analogValues().getPresentValue(0);

    // Write a binary output (e.g. BV:99 = LED)
    bacnet.binaryValues().setPresentValue(99, true);
}
```

---

## BACnet Object Reference

### Analog Values (AV)

| Instance | Name              | Source         | Writable |
|----------|-------------------|----------------|----------|
| AV:0     | ADC0              | A0 (mV)        | No       |
| AV:1     | ADC1              | A1 (mV)        | No       |
| AV:2     | ADC2              | A2 (mV)        | No       |
| AV:3     | ADC3              | A3 (mV)        | No       |
| AV:92    | Device ID         | EEPROM         | Yes      |
| AV:93    | MS/TP Baud        | EEPROM         | Yes      |
| AV:94    | MS/TP MAC         | EEPROM         | Yes      |
| AV:95    | MS/TP Max Manager | EEPROM         | Yes      |
| AV:96    | MCU Frequency     | CPU clock (Hz) | No       |
| AV:97    | CStack Size       | bytes          | No       |
| AV:98    | CStack Unused     | bytes          | No       |
| AV:99    | Uptime            | hours          | No       |

### Binary Values (BV)

| Instance | Name | Arduino pin (Uno)  | Direction |
|----------|------|--------------------|-----------|
| BV:0     | D3   | D3                 | Output    |
| BV:1     | D4   | D4                 | Output    |
| BV:2     | D5   | D5                 | Output    |
| BV:3     | D6   | D6                 | Output    |
| BV:4     | D7   | D7                 | Output    |
| BV:99    | LED  | D13 (built-in LED) | Output    |

To change pins or direction, edit `src/app/bv.c` -> `Object_List[]`.

---

## Build Stats

| Board            | Flash                  | RAM                    |
|------------------|------------------------|------------------------|
| Arduino Uno      | 28 KB / 32 KB (86%)    | 1.6 KB / 2 KB (79%)    |
| STM32 F103C8     | 37 KB / 64 KB (56%)    | 3.7 KB / 20 KB (18%)   |
| ESP32            | 318 KB / 1.3 MB (24%)  | 22 KB / 328 KB (6%)    |
| STM32 F756ZG     | 43 KB / 1 MB (4%)      | 11 KB / 328 KB (3%)    |

---

## License

This repository contains code under two licenses:

### Upstream bacnet-stack (Steve Karg and contributors)
Files under `src/bacnet/` and derived adaptations in `src/app/` and `src/platform/dlmstp.c`
are covered by their original upstream licenses:

- `GPL-2.0-or-later WITH GCC-exception-2.0` — majority of protocol stack files
- `MIT` — selected utility and object files (see per-file `@copyright` tag)

The **GCC Runtime Library Exception** allows the GPL-licensed stack to be linked into firmware without the GPL copyleft extending to the application layer.
Full upstream license: https://github.com/bacnet-stack/bacnet-stack

### Arduino library wrapper (this project)
New files contributed in this repository — including `src/BACnetStack.cpp`, `src/BACnetMSTP.h`,
`src/platform/rs485.cpp`, `src/platform/adc.c`, `src/platform/nvdata.c`,
`src/platform/pin_config.c`, `src/platform/stack.c`, and `src/compile_config.h` —
are released under the **MIT License**.

See [`LICENSE`](LICENSE) for the full MIT text that applies to these new files.
