#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "mcp3008.h"
#include "stats.h"
#include "interpolation.h"

static const char *TAG = "c-coaster";

#define N_SAMPLES 1000

void app_main(void)
{
    mcp3008_init();

    // static keeps these 6 kB off the main task's ~3 kB stack.
    static uint16_t voltages_array[N_SAMPLES];
    static uint16_t scratch[N_SAMPLES];
    static uint16_t stable[N_SAMPLES];
    static uint16_t mass;
    size_t i = 0;

    for (;;) {
        uint16_t raw = mcp3008_read(0);
        // ESP_LOGI(TAG, "FSR raw: %u", raw);
        vTaskDelay(pdMS_TO_TICKS(10));
        if (raw > 10) {
            voltages_array[i++] = raw;
        } else {
            i = 0;
        }

        // ESP_LOGI(TAG, "%zu - ", i);
        if (i == N_SAMPLES) {
            size_t n_stable = stable_counts(voltages_array, N_SAMPLES, 40, 3, scratch, stable);
            if (n_stable == 0) {
                ESP_LOGW(TAG, "no stable readings in batch");
            } else {
                uint16_t avg = mean_u16(stable, n_stable);
                ESP_LOGI(TAG, "kept %zu, avg %u counts", n_stable, avg);
                // spline
                mass = voltageToMass(avg);

                ESP_LOGI(TAG, "Recorded mass: %u", mass);
            }
            i = 0; // the next batch overwrites the buffer, so no need to clear it
        }
    }
}
