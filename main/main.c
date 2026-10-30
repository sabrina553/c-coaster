#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// https://docs.arduino.cc/hardware/nano-esp32/
// GPIO48 is also the onboard LED (D13), so it flickers while clocking.
#define CLK GPIO_NUM_48
#define CS GPIO_NUM_21
// Named from the ESP32's side: MOSI goes to the MCP3008's DIN, MISO from DOUT.
#define MOSI GPIO_NUM_38
#define MISO GPIO_NUM_47

static const char *TAG = "c-coaster";

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
static uint16_t mcp3008_read(uint8_t channel)
{
    uint16_t result = 0;
    gpio_set_level(CS, 0);
    send_bits(0x18 | channel, 5);
    result = receive_bits(11);
    gpio_set_level(CS, 1);
    return result & 0x3FF;
}

static int cmp_u16(const void *a, const void *b)
{
    uint16_t x = *(const uint16_t *)a, y = *(const uint16_t *)b;
    return (x > y) - (x < y);
}

// Median of tail[0..n), sorting a copy in scratch so tail keeps its time order.
static uint16_t median_u16(const uint16_t *tail, size_t n, uint16_t *scratch)
{
    memcpy(scratch, tail, n * sizeof *tail);
    qsort(scratch, n, sizeof *scratch, cmp_u16);
    if (n % 2) {
        return scratch[n / 2];
    }
    return (uint16_t)(((uint32_t)scratch[n / 2 - 1] + scratch[n / 2]) / 2);
}

// Keeps the last tail_pct % of readings within tol_pct % of their median.
static size_t stable_counts(const uint16_t *in, size_t len, unsigned tail_pct, unsigned tol_pct,
                            uint16_t *scratch, uint16_t *out)
{
    if (len == 0) {
        return 0;
    }
    size_t n = (len * tail_pct + 50) / 100; // +50 rounds to nearest
    if (n < 1) {
        n = 1;
    }
    const uint16_t *tail = in + (len - n);
    uint32_t mid = median_u16(tail, n, scratch);

    size_t kept = 0;
    for (size_t i = 0; i < n; i++) {
        uint32_t v = tail[i];
        uint32_t diff = (v > mid) ? v - mid : mid - v;
        if (diff * 100 <= mid * tol_pct) {
            out[kept++] = tail[i];
        }
    }
    return kept;
}

// Integer mean; the sum is 32-bit because 400 * 1023 overflows uint16_t.
static uint16_t mean_u16(const uint16_t *v, size_t n)
{
    if (n == 0) {
        return 0;
    }
    uint32_t sum = 0;
    for (size_t i = 0; i < n; i++) {
        sum += v[i];
    }
    return (uint16_t)((sum + n / 2) / n); // + n/2 rounds to nearest
}

#define N_SAMPLES 1000

void app_main(void)
{
    mcp3008_init();

    // static keeps these 6 kB off the main task's ~3.5 kB stack.
    static uint16_t voltages_array[N_SAMPLES];
    static uint16_t scratch[N_SAMPLES];
    static uint16_t stable[N_SAMPLES];
    size_t i = 0;

    for (;;) {
        uint16_t raw = mcp3008_read(0);
        ESP_LOGI(TAG, "FSR raw: %u", raw);
        vTaskDelay(pdMS_TO_TICKS(200));
        if (raw > 10) {
            voltages_array[i++] = raw;
        }

        ESP_LOGI(TAG, "%u - ", i);
        if (i == N_SAMPLES) {
            size_t n_stable = stable_counts(voltages_array, N_SAMPLES, 40, 3, scratch, stable);
            if (n_stable == 0) {
                ESP_LOGW(TAG, "no stable readings in batch");
            } else {
                uint16_t avg = mean_u16(stable, n_stable);
                ESP_LOGI(TAG, "kept %zu, avg %u counts", n_stable, avg);
            }
            i = 0; // the next batch overwrites the buffer, so no need to clear it
        }
    }
}
