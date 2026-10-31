#include "mcp3008.h"
#include "driver/gpio.h"
#include <assert.h>

// https://docs.arduino.cc/hardware/nano-esp32/
// GPIO48 is also the onboard LED (D13), so it flickers while clocking.
#define CLK GPIO_NUM_48
#define CS GPIO_NUM_21
// Named from the ESP32's side: MOSI goes to the MCP3008's DIN, MISO from DOUT.
#define MOSI GPIO_NUM_38
#define MISO GPIO_NUM_47

void mcp3008_init(void)
{
    // Clear any boot-time function (pull-ups, peripheral routing) first.
    gpio_reset_pin(CS);
    gpio_reset_pin(MOSI);
    gpio_reset_pin(MISO);
    gpio_reset_pin(CLK);

    // CS is active-low: latch it high before enabling the output so the
    // MCP3008 never sees a stray select at boot (MCP3008 datasheet, serial
    // communications).
    gpio_set_level(CS, 1);
    gpio_set_direction(CS, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOSI, GPIO_MODE_OUTPUT);
    gpio_set_direction(MISO, GPIO_MODE_INPUT);
    gpio_set_direction(CLK, GPIO_MODE_OUTPUT);
}

static void clock_pulse(void)
{
    // clock goes high and then low.
    gpio_set_level(CLK, 1);
    gpio_set_level(CLK, 0);
}

static uint16_t receive_bits(int n)
{
    uint16_t result = 0;
    for (int i = n - 1; i >= 0; i--) {
        clock_pulse();
        result = (result << 1) | gpio_get_level(MISO);
    }
    return result;
}
static void send_bits(uint8_t value, int n)
{
    for (int i = n - 1; i >= 0; i--) {
        gpio_set_level(MOSI, (value >> i) & 1);
        clock_pulse();
    }
}

// 130 ksps is recommended for VDD >= 3.3V
uint16_t mcp3008_read(uint8_t channel)
{
    // Only D2..D0 hold the channel; 8+ would corrupt the start/SGL bits (datasheet table 5-2).
    // A bad channel is a programming error, so fail loudly rather than mask it.
    assert(channel < 8);
    uint16_t result = 0;
    gpio_set_level(CS, 0);
    send_bits(0x18 | channel, 5);
    result = receive_bits(11);
    gpio_set_level(CS, 1);
    return result & 0x3FF;
}
