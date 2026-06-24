# BACnetBasicServer — Example

Minimal BACnet MS/TP server for the **BACnetMSTP** Arduino library.  
Supported boards: **Uno, Mega 2560, STM32 F103C8 (Blue Pill), STM32 Nucleo-144 (F756ZG / F746ZG / F429ZI), ESP32, ESP32-S3**.

---

## Quick Start — 3 Steps

### Step 1 — Select your board in the IDE

In the **Arduino IDE**: `Tools → Board → <your board>`  
In **arduino-cli**: use `--fqbn` (see build commands below).

**That's usually enough** — the sketch auto-detects the board and configures the serial port and DE/RE pin automatically. Skip to Step 2.

> **Override manually** only if auto-detect picks the wrong port.  
> Open `BACnetBasicServer.ino` and uncomment exactly **one** of the OPTION blocks (A–F):
>
> | OPTION | Board                              | RS-485 UART | TX pin | RX pin | DE/RE pin |
> |--------|------------------------------------|-------------|--------|--------|-----------|
> | A      | Arduino Uno / Nano / Pro Mini      | `Serial`    | D1     | D0     | D2        |
> | B      | Arduino Mega 2560                  | `Serial1`   | D18    | D19    | D2        |
> | C      | STM32 Blue Pill / Black Pill F103C8| `USART3`    | PB10   | PB11   | PA12      |
> | D      | STM32 Nucleo-144 (F756ZG etc.)     | `USART6`    | PG14   | PG9    | PA12      |
> | E      | ESP32                              | `Serial2`   | GPIO17 | GPIO16 | GPIO18    |
> | F      | ESP32-S3                           | `Serial2`   | GPIO17 | GPIO16 | GPIO2     |
>
> Then comment out all the other OPTION blocks and the `AUTO-DETECT` section.

> **Change the DE/RE pin:** Find your OPTION block in the sketch and change `BACNET_DERE_PIN` to any free GPIO pin connected to your transceiver DE/RE.

---

### Step 2 — Wire the RS-485 transceiver

```
  RS-485 transceiver (MAX485 / SN75176 / SP3485 / etc.)
  ┌──────────────────────────────────────────────────┐
  │  DI  ◄── Arduino TX pin  (see table above)      │
  │  RO  ──► Arduino RX pin  (see table above)      │
  │  DE  ─┐                                         │
  │  RE  ─┴── Arduino DE/RE pin  (BACNET_DERE_PIN)  │
  │  A / B ── RS-485 bus differential pair           │
  └──────────────────────────────────────────────────┘
```

> **Uno/Nano note:** `Serial` (D0/D1) is shared with USB. You **cannot** use the USB serial monitor while the BACnet stack is running. Use a Mega or ESP32 if you need simultaneous debug output.

---

### Step 3 — Build and upload

#### Arduino IDE
1. Select your board under `Tools → Board`.
2. Select the COM port under `Tools → Port`.
3. Click **Upload**.

#### arduino-cli

```powershell
# --- Arduino Uno ---
arduino-cli compile --fqbn arduino:avr:uno `
    --libraries "C:\path\to\Arduino\libraries" "path\to\BACnetBasicServer"
arduino-cli upload -p COM5 --fqbn arduino:avr:uno "path\to\BACnetBasicServer"

# --- Arduino Mega 2560 ---
arduino-cli compile --fqbn arduino:avr:mega `
    --libraries "C:\path\to\Arduino\libraries" "path\to\BACnetBasicServer"

# --- STM32 Blue Pill F103C8 ---
arduino-cli compile `
    --fqbn "STMicroelectronics:stm32:GenF1:pnum=BLUEPILL_F103C8,upload_method=swdMethod" `
    --libraries "C:\path\to\Arduino\libraries" "path\to\BACnetBasicServer"

# --- STM32 Nucleo-144 F756ZG ---
arduino-cli compile `
    --fqbn "STMicroelectronics:stm32:Nucleo_144:pnum=NUCLEO_F756ZG" `
    --libraries "C:\path\to\Arduino\libraries" "path\to\BACnetBasicServer"

# --- ESP32 ---
arduino-cli compile --fqbn esp32:esp32:esp32 `
    --libraries "C:\path\to\Arduino\libraries" "path\to\BACnetBasicServer"

# --- ESP32-S3 ---
arduino-cli compile --fqbn esp32:esp32:esp32s3 `
    --libraries "C:\path\to\Arduino\libraries" "path\to\BACnetBasicServer"
```

---

## Default Network Settings (stored in EEPROM)

| Parameter        | Default  | How to change at runtime               |
|------------------|----------|----------------------------------------|
| MS/TP MAC        | 10       | Write `AV:94` via BACnet WriteProperty |
| Baud rate (bps)  | 38400    | Write `AV:93` via BACnet WriteProperty |
| Max Master       | 127      | Write `AV:95` via BACnet WriteProperty |
| Device Instance  | 260001   | Write `AV:92` via BACnet WriteProperty |

Changes are saved to EEPROM and take effect on next reset.

---

## RS-485 / UART Wiring

```
  RS-485 transceiver (MAX485 / SN75176 / etc.)
  ┌──────────────────────────────────────────────┐
  │  DI  ◄── Arduino TX pin                      │
  │  RO  ──► Arduino RX pin                      │
  │  DE  ─┐                                      │
  │  RE  ─┴── Arduino DE/RE pin (active-HIGH)     │
  │  A / B ── RS-485 bus differential pair        │
  └──────────────────────────────────────────────┘
```

### Pin assignments per board

| Board             | UART for RS-485 | TX pin | RX pin | DE/RE pin (sketch default) |
|-------------------|-----------------|--------|--------|----------------------------|
| **Arduino Uno**   | `Serial` (only UART) | D1 | D0 | D2 (`BACNET_DERE_PIN`) |
| **Arduino Mega**  | `Serial1`       | TX1 (D18) | RX1 (D19) | D2 |
| **STM32 F103C8**  | `Serial_RS485` (USART3) | PB10 | PB11 | PA12 (`PIN_D8`) |
| **ESP32**         | `Serial2`       | GPIO17 | GPIO16 | GPIO18 (`PIN_D8`) |
| **ESP32-S3**      | `Serial2`       | GPIO17 | GPIO16 | GPIO2 (`PIN_D8`) |

> **Uno note:** Because the Uno has only one hardware UART, `Serial` is used for
> RS-485. USB/serial debug output is therefore **not available** while the BACnet
> stack is running.

To change the RS-485 port or DE/RE pin, edit the two `#define` lines near the
top of `BACnetBasicServer.ino`:

```cpp
#define BACNET_RS485_SERIAL   Serial2   // ← your HardwareSerial port
#define BACNET_DERE_PIN       4         // ← your DE/RE Arduino pin number
```

---

## BACnet Object Inventory

### Analog Values (AV) — read via ReadProperty, write via WriteProperty

| BACnet Object | Instance | Name               | Direction  | Arduino pin / source       | Units      |
|---------------|----------|--------------------|------------|----------------------------|------------|
| AV:0          | 0        | ADC0               | Read-only  | A0 (analog input)          | mV         |
| AV:1          | 1        | ADC1               | Read-only  | A1 (analog input)          | mV         |
| AV:2          | 2        | ADC2               | Read-only  | A2 (analog input)          | mV         |
| AV:3          | 3        | ADC3               | Read-only  | A3 (analog input)          | mV         |
| AV:92         | 92       | Device ID          | Read/Write | EEPROM — device instance   | —          |
| AV:93         | 93       | MS/TP Baud         | Read/Write | EEPROM — baud rate (bps)   | bps        |
| AV:94         | 94       | MS/TP MAC          | Read/Write | EEPROM — node address 0–127| —          |
| AV:95         | 95       | MS/TP Max Manager  | Read/Write | EEPROM — max master 0–127  | —          |
| AV:96         | 96       | MCU Frequency      | Read-only  | CPU clock                  | Hz         |
| AV:97         | 97       | CStack Size        | Read-only  | C stack total bytes        | —          |
| AV:98         | 98       | CStack Unused      | Read-only  | C stack headroom bytes     | —          |
| AV:99         | 99       | Uptime             | Read-only  | Updated every 1 s in sketch| hours      |

**How to add your own AV (e.g. a sensor reading):**  
Edit `src/app/av.c` → `Object_List[]`. Add a new row:
```c
{ 10, "MySensor", UNITS_MILLIVOLTS, my_read_fn, NULL, 0.0f },
```
Then write a `static float my_read_fn(void)` that returns the value.

---

### Binary Values (BV) — digital outputs (active-HIGH)

| BACnet Object | Instance | Name | Arduino pin (Uno) | Direction |
|---------------|----------|------|-------------------|-----------|
| BV:0          | 0        | D3   | D3                | Output    |
| BV:1          | 1        | D4   | D4                | Output    |
| BV:2          | 2        | D5   | D5                | Output    |
| BV:3          | 3        | D6   | D6                | Output    |
| BV:4          | 4        | D7   | D7                | Output    |
| BV:99         | 99       | LED  | D13 (built-in LED)| Output    |

**How to change a BV pin:**  
Edit `src/app/bv.c` → `Object_List[]`:
```c
static struct object_data Object_List[] = {
    { 0,  "D3",  PIN_D3,  true },   // ← change PIN_D3 to any Arduino pin number
    { 1,  "D4",  PIN_D4,  true },
    ...
    { 99, "LED", PIN_LED, true }
};
```
Replace `PIN_D3` with a literal pin number (e.g. `9`) or add a new `#define` in
`src/platform/pin_config.h` under your board's section.

**How to make a BV an input instead of output:**  
Change `true` → `false` in the `is_output` column:
```c
{ 0, "D3", PIN_D3, false },   // ← now reads as digital input
```

---

## How to build & upload (arduino-cli)

```powershell
# Compile for Uno
arduino-cli compile --fqbn arduino:avr:uno --libraries "C:\path\to\Arduino\libraries" "path\to\BACnetBasicServer"

# Upload to Uno on COM5
arduino-cli upload -p COM5 --fqbn arduino:avr:uno "path\to\BACnetBasicServer"

# Compile for ESP32
arduino-cli compile --fqbn esp32:esp32:esp32 --libraries "C:\path\to\Arduino\libraries" "path\to\BACnetBasicServer"

# Compile for STM32 Blue Pill F103C8
arduino-cli compile --fqbn "STMicroelectronics:stm32:GenF1:pnum=BLUEPILL_F103C8" --libraries "C:\path\to\Arduino\libraries" "path\to\BACnetBasicServer"
```

---

## Memory usage (reference)

| Board             | Flash        | RAM         | Notes                          |
|-------------------|--------------|-------------|--------------------------------|
| Arduino Uno       | 28 KB / 32 KB (86%) | 1.6 KB / 2 KB (79%) | Stable but near limit |
| Arduino Mega 2560 | ~28 KB / 256 KB (11%) | ~1.6 KB / 8 KB (20%) | Recommended for AVR |
| STM32 F103C8      | 37 KB / 64 KB (56%) | 3.7 KB / 20 KB (18%) | Comfortable |
| ESP32             | 318 KB / 1.3 MB (24%) | 22 KB / 327 KB (6%) | Plenty of headroom |
