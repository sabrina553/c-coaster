#include "time_sync.h"
#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "freertos/FreeRTOS.h"
#include <stdlib.h>

// https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/system/system_time.html
static const char *TAG = "time";

// POSIX TZ for the UK: GMT, BST from last Sunday of March 01:00 to last Sunday of October 02:00.
#define UK_TZ "GMT0BST,M3.5.0/1,M10.5.0"

// Anything before 2024 means the clock is still at its 1970 boot value.
#define MIN_VALID_YEAR 2024

bool time_sync_init(uint32_t timeout_ms)
{
    setenv("TZ", UK_TZ, 1);
    tzset();

    esp_sntp_config_t cfg = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    ESP_ERROR_CHECK(esp_netif_sntp_init(&cfg));
    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(timeout_ms)) != ESP_OK) {
        ESP_LOGW(TAG, "no sync after %lu ms", (unsigned long)timeout_ms);
        return false;
    }
    ESP_LOGI(TAG, "synced");
    return true;
}

bool time_is_synced(void)
{
    time_t now = time(NULL);
    struct tm t;
    localtime_r(&now, &t);
    return t.tm_year + 1900 >= MIN_VALID_YEAR;
}

int32_t time_day_key(time_t now)
{
    struct tm t;
    localtime_r(&now, &t);
    return (t.tm_year + 1900) * 10000 + (t.tm_mon + 1) * 100 + t.tm_mday;
}
