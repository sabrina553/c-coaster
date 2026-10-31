#pragma once
#include <stdint.h>

// Bit-banged SPI driver for the MCP3008 10-bit ADC.
void mcp3008_init(void);
uint16_t mcp3008_read(uint8_t channel); // returns 0..1023

#include "driver/gpio.h"

// https://docs.arduino.cc/hardware/nano-esp32/
// GPIO48 is also the onboard LED (D13), so it flickers while clocking.
#define CLK GPIO_NUM_48
#define CS GPIO_NUM_21
// Named from the ESP32's side: MOSI goes to the MCP3008's DIN, MISO from DOUT.
#define MOSI GPIO_NUM_38
#define MISO GPIO_NUM_47
