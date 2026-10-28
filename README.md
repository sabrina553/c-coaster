# c-coaster

[SmartCoaster](../archive/SmartCoaster) (Raspberry Pi + Python) rewritten in embedded C for an Arduino Nano ESP32 (ESP32-S3) with ESP-IDF.

## Build and flash

```zsh
idf                       # alias: loads ESP-IDF into the shell
idf.py set-target esp32s3 # once
idf.py build
idf.py -p /dev/ttyACM0 flash monitor   # Ctrl+] quits the monitor
```

## Hardware
FSR + 10 kΩ divider into an MCP3008 (powered at **3.3 V**, the ESP32 is not 5 V tolerant) over SPI; six status LEDs.

## What I learnt
- **Serial port access:** `/dev/ttyACM0` needs the `dialout` group (`sudo usermod -aG dialout "${USER}"`, then log in again).
- **Download mode:** B1 (GPIO0) is a strapping pin. If it's low at reset, the chip stays in the ROM bootloader ("waiting for download"). Only jumper it to GND when a flash can't connect.
- **USB reconnects:** the S3's USB is on-chip, so every reset briefly disconnects the monitor. That's expected.
- **`vTaskDelay`, not busy loops:** yielding keeps the FreeRTOS task watchdog happy.

## What I would do better
