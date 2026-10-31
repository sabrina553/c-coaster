#pragma once
#include <stdint.h>

// Bit-banged SPI driver for the MCP3008 10-bit ADC.
void mcp3008_init(void);
uint16_t mcp3008_read(uint8_t channel); // returns 0..1023
