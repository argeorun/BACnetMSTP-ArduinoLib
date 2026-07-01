# BACnetMSTP — Arduino Library

A fully hardware-validated BACnet MS/TP server library for Arduino.
Connects Arduino-compatible boards to RS-485 BACnet networks with
a single include and four lines of sketch code.

---

## Based On

This library is derived from
**[BACnet-MSTP-Arduino](https://github.com/argeorun/BACnet-MSTP-Arduino)**
by the same author, which itself adapts the open-source
**[bacnet-stack](https://github.com/bacnet-stack/bacnet-stack)**
by Steve Karg and contributors.

The upstream bacnet-stack provides the complete BACnet protocol stack
(encoding/decoding, MS/TP state machine, service handlers).

See [`src/bacnet/`](src/bacnet/) for upstream files.
See [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) for full attribution.

---

## Features

- **BACnet MS/TP** over RS-485 (configurable baud rate and MAC address)
- **ReadProperty**, **WriteProperty**, **Who-Is / I-Am** services
- **Analog Value objects** (AV x12): ADC inputs, device config, diagnostics
- **Binary Value objects** (BV x6): digital outputs + built-in LED
- **EEPROM-backed** device instance, MAC address, baud rate
- **Single include** — `#include <BACnetMSTP.h>`, four-line sketch
- Validated with **YABE BACnet Explorer** over RS-485

---

## Board Support

| Board | Status | RS-485 UART | TX | RX | DE/RE |
|---|---|---|---|---|---|
| Arduino Uno R3 | Tested | `Serial` | D1 | D0 | D2 |
| Arduino Mega 2560 | Tested | `Serial1` | D18 | D19 | D2 |
| ESP32 (generic dev kit) | Tested | `Serial2` | GPIO17 | GPIO16 | GPIO4 |
| ESP32-S3 | Tested | `Serial2` | GPIO17 | GPIO16 | GPIO2 |
| STM32 Nucleo-144 F756ZG | Tested | `Serial6` | PG14 | PG9 | PA12 |
| STM32 Blue Pill F103CB (compiled, not yet hardware tested) | Compile only | `Serial3` | PB10 | PB11 | PA12 |

---

## Installation

### Arduino Library Manager (recommended)
1. Open Arduino IDE -> **Sketch -> Include Library -> Manage Libraries...**
2. Search `BACnetMSTP`
3. Click **Install**

### Manual (zip)
1. Download this repository as a zip
2. Arduino IDE -> **Sketch -> Include Library -> Add .ZIP Library...**
3. Select the downloaded zip

---

## Quick Start

```cpp
#include <BACnetMSTP.h>

BACnetDevice device;

void setup() {
    device.begin();   // init RS-485, load EEPROM config, start MS/TP
}

void loop() {
    device.update();  // receive frames, handle requests
}
```

On first boot, factory defaults are written to EEPROM:
MAC = 10, baud = 38400, device instance = 260001.

Wire your RS-485 transceiver to the TX, RX, and DE/RE pins shown in the
Board Support table above.

Discover the device with a BACnet client (YABE, NiagaraAX, etc.) on the
same RS-485 network.

---

## API

```cpp
device.AV(92).get();          // read AV instance 92 (float)
device.AV(92).set(260005);    // write AV instance 92

device.BV(99).get();          // read BV instance 99 (BACNET_BINARY_PV)
device.BV(99).set(true);      // write BV instance 99
```

---

## BACnet Object Reference

### Analog Values (AV)

| Instance | Name | Source | Writable | UNO |
|---|---|---|---|---|
| AV:0 | ADC0 | A0 (mV) | No | - |
| AV:1 | ADC1 | A1 (mV) | No | - |
| AV:2 | ADC2 | A2 (mV) | No | - |
| AV:3 | ADC3 | A3 (mV) | No | - |
| AV:92 | Device ID | EEPROM | Yes | Yes |
| AV:93 | MS/TP Baud | EEPROM | Yes | Yes |
| AV:94 | MS/TP MAC | EEPROM | Yes | Yes |
| AV:95 | MS/TP Max Manager | EEPROM | Yes | Yes |
| AV:96 | MCU Frequency | CPU clock (Hz) | No | - |
| AV:97 | CStack Size | bytes | No | - |
| AV:98 | CStack Unused | bytes | No | - |
| AV:99 | Uptime | hours | No | Yes |

UNO column: Yes = present on Uno, - = excluded (RAM constraint).

### Binary Values (BV)

| Instance | Name | Pin | Direction |
|---|---|---|---|
| BV:0 | D3 | D3 | Output |
| BV:1 | D4 | D4 | Output |
| BV:2 | D5 | D5 | Output |
| BV:3 | D6 | D6 | Output |
| BV:4 | D7 | D7 | Output |
| BV:99 | LED | Built-in LED | Output |

---

## Build Stats (v2.0.0)

| Board | Flash | RAM |
|---|---|---|
| Arduino Uno R3 | 26224B / 32KB (81%) | 1308B / 2KB (63%) |
| Arduino Mega 2560 | 28584B / 256KB (11%) | 1619B / 8KB (19%) |
| ESP32 generic | 324268B / 1.3MB (24%) | 23212B / 328KB (7%) |
| ESP32-S3 | 356340B / 1.3MB (27%) | 23288B / 328KB (7%) |
| Blue Pill STM32F103CB | 31572B / 128KB (24%) | 3528B / 20KB (17%) |
| Nucleo-144 STM32F756ZG | 37356B / 1MB (3%) | 11028B / 328KB (3%) |

---

## Known Limitations

- Blue Pill (STM32F103CB): compiled and passes build, hardware test pending
- Segmentation not supported (single-frame APDU only, MAX_APDU = 128 bytes)
- Client services (SubscribeCOV, ReadPropertyMultiple) not implemented — server only
- Arduino Uno: ADC and diagnostics AV objects excluded due to RAM constraint (2KB)

---

## Breaking Change vs v1.x

The v2.0.0 API is completely different from v1.x.

| | v1.x | v2.0.0 |
|---|---|---|
| Include | `BACnetMSTP.h` | `BACnetMSTP.h` (same) |
| Object | `BACnetMSTP bacnet(Serial1, 2)` | `BACnetDevice device` |
| Init | `bacnet.begin()` | `device.begin()` |
| Loop | `bacnet.task()` | `device.update()` |
| Read AV | `bacnet.analogValues().getPresentValue(0)` | `device.AV(0).get()` |
| Write BV | `bacnet.binaryValues().setPresentValue(99, true)` | `device.BV(99).set(true)` |

Existing v1.x sketches must be updated to the new API before upgrading.

---

## License

This repository contains code under two licenses.

**Upstream bacnet-stack** (Steve Karg and contributors):
files under `src/bacnet/` and derived adaptations in `src/platform/dlmstp.c`
retain their original upstream licenses (GPL-2.0-or-later WITH GCC-exception-2.0 or MIT).

**Arduino library wrapper** (this project):
new files contributed here are released under the **MIT License**.
Copyright (c) 2025-2026 George Arun.

See [`LICENSE`](LICENSE) and [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).
