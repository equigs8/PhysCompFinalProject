# Hardware & Embedded Systems Review (`PhysCompFinalProject`)

This document details the hardware, pinout, electrical, and embedded firmware pitfalls identified across the firmware ([`main.cpp`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp)), the Wokwi simulation ([`diagram.json`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/diagram.json)), and the KiCad PCB design ([`FinalProjectPCB.kicad_sch`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject_PCB/FinalProjectPCB.kicad_sch) / [`FinalProjectPCB.kicad_pcb`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject_PCB/FinalProjectPCB.kicad_pcb)).

---

## 1. Critical Mismatch: KiCad PCB Uses `ESP32-S2-DevKitC-1` Instead of `ESP32-S3-DevKitC-1`

### The Problem
In [`FinalProjectPCB.kicad_sch`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject_PCB/FinalProjectPCB.kicad_sch#L5648-L5692), the microcontroller module `U1` is placed using the **`PCM_Espressif:ESP32-S2-DevKitC-1`** symbol and footprint, whereas [`platformio.ini`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/platformio.ini#L11-L13), [`README.md`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/README.md#L8), and [`diagram.json`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/diagram.json#L6) all target the **`ESP32-S3-DevKitC-1`**.

Although both DevKits use 44-pin (2×22) headers, **their pinouts are not identical**:

| Physical Header Pin (`U1`) | PCB Net Name | `ESP32-S2-DevKitC-1` Pin (Schematic) | Actual `ESP32-S3-DevKitC-1` Pin | Impact when plugging an ESP32-S3 into the PCB |
| :--- | :--- | :--- | :--- | :--- |
| Left Pin 14 | `PowerLED` | `GPIO46` (Input-only on S2) | **`GPIO3`** (JTAG / Strapping Pin) | Firmware expects `GPIO46`, which is on Left Pin 16 on the S3. Driving `GPIO3` affects a strapping pin. |
| Right Pin 28 (J3-17) | `CENTER` | `GPIO33` | **`GPIO47`** | Center button of `SW10` routes to `GPIO47` on S3, not `GPIO33`. |
| Right Pin 29 (J3-16) | `LEFT` | `GPIO34` | **`GPIO48` (Onboard RGB LED)** | `GPIO48` on the ESP32-S3-DevKitC-1 v1.1 drives the onboard WS2812 RGB LED. |
| Right Pin 30 (J3-15) | `UP` | `GPIO45` | **`GPIO45` (Strapping Pin)** | Boot strapping conflict (see Section 3). |

### Potential Fixes
1. **In Firmware (No PCB respin needed):** Update your pin definitions in [`main.cpp`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L58-L78) to match the *actual* `ESP32-S3-DevKitC-1` GPIOs that land on those physical PCB header pins (e.g., `PIN_POWER_INDICATOR` is on `GPIO3` if using the existing PCB trace, and `PIN_CENTER` is on `GPIO47`).
2. **In KiCad (For future PCB revisions):** Replace `U1` with the `ESP32-S3-DevKitC-1` symbol/footprint and update the schematic reference table ([lines 3528–4140](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject_PCB/FinalProjectPCB.kicad_sch#L3528-L4140)), which currently lists outdated pin numbers (`UP = GPIO 43`, `LEFT = GPIO 44`).

---

## 2. Three-Way Pinout Discrepancy (`main.cpp` vs. `diagram.json` vs. PCB)

### The Problem
The pin assignments in [`main.cpp`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L53-L82), [`diagram.json`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/diagram.json#L83-L114), and [`FinalProjectPCB.kicad_sch`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject_PCB/FinalProjectPCB.kicad_sch) disagree on several inputs and peripherals:

| Signal / Component | `main.cpp` | `diagram.json` (Wokwi) | `FinalProjectPCB` (Actual Hardware) | Conflict / Issue |
| :--- | :--- | :--- | :--- | :--- |
| **Select Button (`SW9`)** | `GPIO 16` | `GPIO 39` | `GPIO 39` (Pin 36) | **Broken on PCB & Wokwi:** Firmware reads `GPIO 16`, so pressing Select on `GPIO 39` is never detected. |
| **Button B (`SW5`)** | `GPIO 17` | `GPIO 42` | `GPIO 42` (Pin 39) | **Broken on PCB & Wokwi:** Firmware reads `GPIO 17`, so pressing Button B on `GPIO 42` is never detected. |
| **Nav Center (`SW10`)** | *Undefined* | *Not present* | `GPIO 47` (Pin 28) | The 5-way switch center click is wired on the PCB, but unused in code. |
| **Serial Link TX (`J1`)** | `GPIO 16` | *Not wired* | `GPIO 43 / U0TXD` (Pin 43) | In `main.cpp`, `SERIAL_TX_PIN` (`16`) collides with `PIN_SELECT` (`16`). |
| **Serial Link RX (`J1`)** | `GPIO 17` | *Not wired* | `GPIO 44 / U0RXD` (Pin 42) | In `main.cpp`, `SERIAL_RX_PIN` (`17`) collides with `PIN_BUTTONB` (`17`). |
| **Volume Slider (`RV1`)** | `GPIO 8` | `GPIO 8` | `GPIO 21` (Pin 27) | **Hardware Bug:** `GPIO 21` on ESP32-S3 has **no ADC** (see Section 4). |
| **TFT `MOSI` / `SCK`** | Hardware `SPI` (`11` / `12`) | Wired to *both* `11`/`12` and `23`/`18` | `GPIO 11` / `GPIO 12` | Wokwi shorts two pairs of GPIOs together; code comments wrongly say `18`/`23`. |
| **TFT `RST`** | `-1` | `GPIO 14` | `ESP32 RST` (Pin 3) | Minor mismatch between Wokwi (`GPIO 14`) and PCB (`RST`). |

### Potential Fixes
* Change [`main.cpp`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L63-L66) so `PIN_SELECT` and `PIN_BUTTONB` match both the PCB and Wokwi:
  ```cpp
  #define PIN_SELECT  39
  #define PIN_BUTTONB 42
  #define PIN_CENTER  47 // Optional: 5-way nav center click on PCB
  ```
* Remove the duplicate SPI wires in [`diagram.json`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/diagram.json#L88-L89) (`esp:18` and `esp:23`—note that `GPIO 23` does not exist on the ESP32-S3).

---

## 3. ESP32-S3 Boot Strapping Pins Used by Peripherals (`GPIO 45`, `GPIO 3`, `GPIO 46`)

### The Problem
On the ESP32-S3, **GPIO 0, GPIO 3, GPIO 45, and GPIO 46** are hardware strapping pins sampled during chip reset:
1. **`PIN_UP` on `GPIO 45` ([`main.cpp` line 59](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L59)):**
   `GPIO 45` selects the `VDD_SPI` flash/PSRAM voltage (`0` = 3.3 V, `1` = 1.8 V). It has an internal weak pull-down, and `PIN_UP` is wired as an active-LOW button to `GND`, which is safe when pressed, **provided you never add an external pull-up resistor** or enable `INPUT_PULLUP` before boot completes. However, on `esp32-s3-devkitc1-n8r8` (which uses 3.3V Octal PSRAM/Flash), pulling `GPIO 45` HIGH during reset will prevent the board from booting.
2. **`PowerLED` on Left Pin 14 (`GPIO 3` on S3, labeled `GPIO 46` on S2):**
   On the PCB, `PowerLED` (`D1` + `R1` to `GND`) is wired to physical Pin 14 (`GPIO 3` on the ESP32-S3), which is the `JTAG` signal source strapping pin.

### Potential Fixes
* **Software Safeguard:** If keeping `PIN_UP` on `GPIO 45` for the existing PCB, be aware that `pinMode(PIN_UP, INPUT_PULLUP)` pulls `GPIO 45` HIGH *while the sketch is running*. If the user presses the hardware `RST` button or triggers `ESP.restart()` before the pin resets, verify that the internal pull-up is released or burn the `VDD_SPI` eFuse (`espefuse.py set_flash_voltage 3.3V`) so the ESP32-S3 ignores `GPIO 45` at boot.
* **PCB Revision:** Move `UP` and `PowerLED` off strapping pins onto free general-purpose pins such as `GPIO 6`, `GPIO 8`, `GPIO 9`, `GPIO 10`, `GPIO 15`, `GPIO 16`, or `GPIO 17`.

---

## 4. Volume Potentiometer (`RV1`) Wired to a Non-ADC Pin (`GPIO 21`) on PCB

### The Problem
On the PCB ([`FinalProjectPCB.kicad_sch`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject_PCB/FinalProjectPCB.kicad_sch#L4072)), the wiper of the volume potentiometer `RV1` (`VOLUME`) is routed to **Pin 27 (`GPIO 21`)**.
* On the ESP32-S3, **ADC1** only covers **`GPIO 1` – `GPIO 10`**, and **ADC2** only covers **`GPIO 11` – `GPIO 20`**.
* **`GPIO 21` has no analog-to-digital converter (ADC) hardware.** Calling `analogRead(21)` in [`updateVolume()`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L343-L351) will fail and issue an error (`GPIO 21 is not an ADC pin`).
* Furthermore, if ESP-NOW / Wi-Fi is active, **ADC2 (`GPIO 11` – `GPIO 20`) cannot be used reliably** because the Wi-Fi radio shares the ADC2 arbiter.

### Potential Fixes
* **For the Existing PCB (Bodge Wire Fix):** Cut the trace going to Pin 27 (`GPIO 21`) or leave Pin 27 unpopulated on the header, and run a jumper wire from the `RV1` wiper pad to an **ADC1** pin such as **`GPIO 8`** (Left Pin 12) or **`GPIO 6`** (Left Pin 6). Then set `#define PIN_VOLUME 8` in [`main.cpp`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L81).
* **For a New PCB Revision:** Route the `VOLUME` net to `GPIO 8` (`ADC1_CH7`) so it works simultaneously with Wi-Fi / ESP-NOW and matches [`diagram.json`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/diagram.json#L113).

---

## 5. Wired Serial Link (`J1`) Conflicts with USB UART0 & Debug Prints

### The Problem
There are three stacked issues with the wired multiplayer Chess connection:
1. **PCB uses `UART0` (`GPIO 43` TX / `GPIO 44` RX):**
   In [`FinalProjectPCB.kicad_sch`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject_PCB/FinalProjectPCB.kicad_sch#L4362-L4383), the 3-pin Serial Jack `J1` is connected to `GPIO 43` (`U0TXD`) and `GPIO 44` (`U0RXD`). On the `ESP32-S3-DevKitC-1`, `UART0` is hardwired to the onboard USB-to-UART bridge chip (`CP2102N` / `CH343`).
2. **Debug `Serial.println()` Corrupts the Game Protocol:**
   Because [`sendRemoteMove()`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L1842-L1852) and [`receiveChessMove()`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L1869-L1877) use `Serial` (`UART0`), every debug print in [`handleGeneralInput()`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L392-L434) (such as `Serial.println("Home Button Pressed")`) is transmitted across the link cable. Since `"Home Button Pressed"` contains `'M'` (or if any debug string contains `'M'`), the other console's `if (Serial.read() == 'M')` parser will read the next two ASCII characters as `fromIdx` and `toIdx` and execute a garbage chess move!
3. **Straight-Through Connector Pinout:**
   On `J1` (`Conn_01x03`), Pin 1 is `GND`, Pin 2 is `TX`, and Pin 3 is `RX`. Two identical PCBs connected with a straight 1-to-1 cable will connect `TX -> TX` and `RX -> RX` instead of crossing `TX -> RX`.

### Potential Fixes
* **Cable Fix:** Ensure the 3-wire link cable between two consoles crosses Pin 2 and Pin 3 (`TX <-> RX`, `GND <-> GND`).
* **Software Fix (If using `J1` on `GPIO 43`/`44`):** Remove or guard all `Serial.println()` debug statements behind a `#ifdef DEBUG_SERIAL` macro so only binary move packets are sent over `Serial`, and add a simple checksum/framing byte:
  ```cpp
  // Frame: [0xAA, 'M', fromIdx, toIdx, checksum]
  ```
* **Hardware + Software Fix (Recommended):** Route `J1` to `GPIO 16` (`RX`) and `GPIO 17` (`TX`) using **`Serial2`** (`Serial2.begin(9600, SERIAL_8N1, 16, 17)`), keeping `Serial` (`UART0`) exclusively for USB debugging. Update [`sendRemoteMove()`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L1842-L1852) and [`receiveChessMove()`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L1869-L1877) to call `Serial2.write()` and `Serial2.read()`.

---

## 6. Electromagnetic Transducer (`AT-1224-TWT-5V-2-R`) Driven Directly from GPIO 7

### The Problem
According to [`README.md`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/README.md#L26) and [`FinalProjectPCB.kicad_sch`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject_PCB/FinalProjectPCB.kicad_sch#L6201), `LS1` is a PUI Audio **`AT-1224-TWT-5V-2-R`**.
* Despite being labeled `Piezo` in the schematic, the `AT-1224-TWT-5V-2-R` is actually a **5V electromagnetic (coil-driven) audio transducer** with a $160\,\Omega$ coil resistance and a $20\text{–}30\text{ mA}$ current draw.
* Because it contains an inductive coil rather than a capacitive piezo element, switching it directly from `GPIO 7` without a **flyback (freewheeling) diode** generates negative/positive inductive voltage spikes (back-EMF) when the PWM signal switches off, which can damage the ESP32-S3 GPIO pad ESD diodes over time.

### Potential Fixes
* **Immediate Protection (On Existing PCB):** Solder a small signal diode (e.g., `1N4148` or `1N5819` Schottky) in **reverse-bias** across the `LS1` pads (Cathode to `GPIO 7` / `P`, Anode to `GND` / `N`) to clamp inductive flyback spikes.
* **Proper Driver Circuit (For PCB Revision):** Drive `LS1` through an NPN transistor (e.g., `2N3904` / `2N2222`) or N-channel MOSFET (`2N7000`) with a flyback diode across the transducer, or swap `LS1` for a true passive capacitive piezo element (which does not have an inductive coil).
* **ESP32 Arduino Core 3.x LEDC API Note:** [`initAudio()`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L338-L341) and [`playTone()`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L353-L364) use `ledcAttach(PIN_PIEZO, 2000, AUDIO_RESOLUTION)`. When uncommenting audio, note that calling `ledcWriteTone(PIN_PIEZO, freq)` resets the duty cycle to 50% (128), so `ledcWrite(PIN_PIEZO, currentVolume)` must be called *after* `ledcWriteTone()` (as your commented code currently does), and `delay(duration)` should be replaced with a non-blocking `millis()` timer so audio doesn't freeze button inputs.

---

## 7. Power Supply Architecture (`BT1` -> `5V-IN` Pin)

### The Problem
In [`README.md`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/README.md#L40) and [`FinalProjectPCB.kicad_sch`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject_PCB/FinalProjectPCB.kicad_sch#L6386-L6452), battery `BT1` connects through power switch `SW7` directly into the `5V` pin (Pin 21) of the DevKit, with:
1. **No reverse-polarity protection** (if `BT1` is wired into the screw terminal backwards, the DevKit LDO and ESP32-S3 will be destroyed instantly).
2. **No bulk decoupling capacitor** on the PCB's `5V-IN` or `3V3` rails to handle current spikes from Wi-Fi / ESP-NOW transmission bursts (~350 mA peaks) and the ILI9341 backlight.
3. **Voltage Margin with 4×AA Batteries:** Four fresh alkaline AA cells in series measure **$6.0\text{V} – 6.4\text{V}$** under light load. Feeding $6.4\text{V}$ into the `5V` pin forces the DevKit's linear 3.3V LDO regulator to drop $>3.1\text{V}$ as heat ($P = (6.4\text{V} - 3.3\text{V}) \times I$), and risks back-feeding if a USB cable is plugged in while the battery switch `SW7` is ON.

### Potential Fixes
* **Use 3×AA with a 5V Boost or 4×NiMH Rechargeable AA ($4.8\text{V} – 5.4\text{V}$):** Using 4× NiMH AA cells ($1.2\text{V}$ nominal, $4.8\text{V}$ total) or a USB power bank / LiPo + boost converter (such as the Adafruit PowerBoost 1000C linked in [`README.md`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/README.md#L21)) keeps the `5V` rail safely regulated at $5.0\text{V}$.
* **Add a Series Schottky Diode (`1N5817`) and Bulk Capacitor ($100\,\mu\text{F}$):** Place a Schottky diode in series with `BT1+` to prevent reverse-polarity damage and prevent USB $5\text{V}$ from charging alkaline batteries when USB is connected.

---

## 8. SPI Display Rendering Performance & Thread Safety

### The Problem
1. **Pixel-by-Pixel SPI Transactions:**
   [`drawSpriteWithTransparency()`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L197-L209) calls `tft.drawPixel(x + i, y + j, color)` inside a double loop. Every non-transparent pixel initiates a separate SPI transaction (assert CS, send column command + 4 bytes, send row command + 4 bytes, send pixel command + 2 bytes, deassert CS = **13+ bytes of overhead for 2 bytes of pixel data**).
2. **ESP-NOW Interrupt/Task Concurrency:**
   [`OnDataRecv()`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L146-L154) runs in the high-priority Wi-Fi task on Core 0, while `loop()` runs on Core 1. [`wirelessMoveReceived`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L144) and [`incomingMove`](file:///C:/Users/Ethan/Desktop/MyStuff/PhysCompFinalProject/FinalProject/src/main.cpp#L143) are not declared `volatile`.

### Potential Fixes
* **Wrap `drawSpriteWithTransparency` in an SPI transaction (`startWrite` / `endWrite`):**
  ```cpp
  void drawSpriteWithTransparency(int x, int y, const uint16_t *bitmap, int w, int h, uint16_t transparentColor) {
      tft.startWrite();
      for (int j = 0; j < h; j++) {
          for (int i = 0; i < w; i++) {
              uint16_t color = pgm_read_word(&bitmap[j * w + i]);
              if (color != transparentColor) {
                  tft.writePixel(x + i, y + j, color);
              }
          }
      }
      tft.endWrite();
  }
  ```
  Switching from `tft.drawPixel()` to `tft.startWrite()` + `tft.writePixel()` + `tft.endWrite()` keeps CS asserted and eliminates redundant SPI transaction lock/unlock overhead on every pixel.
* **Mark shared ESP-NOW variables `volatile`:**
  ```cpp
  volatile bool wirelessMoveReceived = false;
  ```
