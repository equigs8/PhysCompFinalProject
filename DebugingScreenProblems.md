# Debugging Screen Problems (ILI9341 + ESP32-S3)

When an ILI9341 + ESP32-S3 project stops showing an image after sitting for several months, the cause depends on whether the screen is **glowing white (backlit with no graphics)** or **completely dark**.

---

## 1. Screen is Glowing White (Backlight ON, No Picture)

### A. Power-On Reset Timing (`TFT_RST = -1` vs. PCB `RESET` Wiring)

- **Why it happens:** In [`main.cpp`](FinalProject/src/main.cpp#L55), `TFT_RST` is set to `-1`, while on the PCB (`J2` Pin 4), the screen's `RESET` pin is tied to the ESP32's hardware **`RST`** pin. When you flip the power switch, the ESP32 boots and runs `tft.begin()` immediately—often before the display's 3.3V power rail and internal charge pumps have stabilized. Because `TFT_RST` is `-1`, the library cannot toggle a GPIO to hardware-reset the screen after power stabilizes.
- **How to test:** Power on the device, wait 1 second, and press the physical **`RST` button** on the ESP32-S3-DevKitC-1 board.
- **Fix in `setup()`:** Add a power-stabilization delay *before* `tft.begin()`:

```cpp
delay(500); // Let 3.3V rail and ILI9341 stabilize after power-on
SPI.begin(12, -1, 11, TFT_CS); // Explicitly bind SCK=12, MISO=-1, MOSI=11, CS=5
tft.begin(16000000); // Use 16 MHz SPI clock instead of 40 MHz default
```

### B. 40 MHz Default SPI Clock + Oxidized/Loose Terminal Block Contacts (`J2`)

- **Why it happens:** On the PCB, the display connects through a 9-pin screw terminal block (`J2`). After sitting for months, screw terminal contacts relax slightly or build up thin oxidation. `Adafruit_ILI9341` defaults to a **40 MHz SPI clock** on the ESP32, which is very sensitive to wire capacitance and contact resistance. If **any one** of `CS` (`GPIO 5`), `DC/RS` (`GPIO 4`), `MOSI` (`GPIO 11`), `SCK` (`GPIO 12`), or `RESET` (`RST`) has a loose connection, the backlight (`VCC`/`LED` on Pins 1 & 8) will still light up the screen white, but no image will appear.
- **How to test:** Re-tighten all 8 used screws on `J2` (`Pin 1` through `Pin 8`), reseat the display header pins, and lower the SPI clock to 16 MHz (`tft.begin(16000000)`).

### C. ESP32-S3 Boot Loop from PSRAM / Flash Configuration (If Reflashed)

- **Why it happens:** Your [`platformio.ini`](FinalProject/platformio.ini#L11-L13) targets `board = esp32-s3-devkitc1-n8r8`, which compiles with **Octal PSRAM (`qio_opi`)** enabled (`BOARD_HAS_PSRAM`). If the physical ESP32-S3 board installed in your console is actually an `ESP32-S3-WROOM-1-N8` (no PSRAM) or `N8R2` (Quad PSRAM) and you re-uploaded the code recently, the ESP32-S3 will crash in an endless boot loop before ever reaching `setup()`.
- **How to test:** Plug the ESP32-S3 into your computer via the **UART** USB port, open the PlatformIO Serial Monitor at **`115200` baud** (the hardware ROM bootloader baud rate) or **`9600` baud** (your sketch's `SERIAL_BAUD_RATE`), and press `RST`:
  - Do you see `"Starting up..."` at `9600` baud?
  - Or do you see `Octal Flash option selected, but EFUSE not configured` / `Guru Meditation Error` / `rst:0x...` repeating at `115200` baud?

### D. Strapping Pin Held by the Case (`GPIO 45` / `GPIO 0` / `GPIO 3`)

- **Why it happens:** If the 3D-printed case is pressing down on the D-pad or a button is shorted, holding a strapping pin during power-up can prevent the ESP32-S3 from booting.

---

## 2. Screen is Completely Dark (Backlight OFF)

- **Drained Batteries (`BT1`) or Oxidized Power Switch (`SW7`):**
  The display's `VCC` (Pin 1) and `LED` backlight (Pin 8) are powered from the ESP32's `3V3` regulator output. If the AA batteries have drained over the past few months—or if `SW7` has high contact resistance—the `3V3` rail won't come up.

> [!WARNING]
> In your latest [`main.cpp`](FinalProject/src/main.cpp#L2335-L2336), `PIN_POWER_INDICATOR` is commented out (and wired to `GPIO 3` on the PCB instead of `GPIO 46`), so **your external Power LED (`D1`) will stay OFF even when the board is powered**. Check the small onboard LED directly on the ESP32-S3 DevKit instead, or power the ESP32-S3 directly via USB.

---

## 3. Diagnostic Checklist

1. When you turn it on (or plug it into USB), **does the screen light up white, or stay completely dark?**
2. If you plug it into USB and open the Serial Monitor (at `9600` baud), **does it print `"Starting up..."` and button press messages when you click the buttons?**
