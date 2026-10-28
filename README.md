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

## What I would do better
