#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Nano ESP32 onboard LED (D13) is GPIO48, see the Arduino Nano ESP32 pinout:
// https://docs.arduino.cc/hardware/nano-esp32/
#define LED_GPIO GPIO_NUM_48
#define BLINK_MS 500

static const char *TAG = "c-coaster";

void app_main(void)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    uint32_t level = 0;
    for (;;) {
        level ^= 1u;
        gpio_set_level(LED_GPIO, level);
        ESP_LOGI(TAG, "LED %s", level ? "on" : "off");
        // vTaskDelay yields to the scheduler; a busy loop would trip the task watchdog.
        vTaskDelay(pdMS_TO_TICKS(BLINK_MS));
    }
}
