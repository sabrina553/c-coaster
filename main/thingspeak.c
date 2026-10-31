#include "thingspeak.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <time.h>

// https://www.mathworks.com/help/thingspeak/writedata.html
static const char *TAG = "thingspeak";
#define URL "https://api.thingspeak.com/update"
// Free tier accepts one update per 15 s; anything faster is silently dropped.
#define MIN_INTERVAL_US (15 * 1000000LL)

bool thingspeak_send(uint16_t counts, uint16_t mass, uint32_t daily_total, time_t timestamp)
{
    static int64_t last_us = -MIN_INTERVAL_US;
    int64_t now_us = esp_timer_get_time();
    if (now_us - last_us < MIN_INTERVAL_US) {
        ESP_LOGW(TAG, "rate-limited, skipping");
        return false;
    }

    char body[192];
    int len = snprintf(body, sizeof body, "api_key=%s&field1=%u&field2=%u&field3=%lu",
                       CONFIG_COASTER_THINGSPEAK_API_KEY, counts, mass,
                       (unsigned long)daily_total);
    if (timestamp != 0 && len > 0 && (size_t)len < sizeof body) {
        // created_at keeps the reading's own time if the send is delayed.
        struct tm t;
        gmtime_r(&timestamp, &t);
        size_t added = strftime(body + len, sizeof body - (size_t)len,
                                "&created_at=%Y-%m-%dT%H:%M:%SZ", &t);
        // Unlike snprintf, strftime returns 0 when it doesn't fit.
        len = added ? len + (int)added : (int)sizeof body;
    }
    // snprintf returns the length it *wanted*; >= sizeof body means it was cut off.
    if (len < 0 || (size_t)len >= sizeof body) {
        ESP_LOGE(TAG, "request body truncated, not sending");
        return false;
    }

    esp_http_client_config_t cfg = {
        .url = URL,
        .method = HTTP_METHOD_POST,
        .crt_bundle_attach = esp_crt_bundle_attach, // verify the server's TLS cert
        .timeout_ms = 10000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
    esp_http_client_set_post_field(client, body, len);

    esp_err_t err = esp_http_client_perform(client);
    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    // Never log body: it contains the API key.
    if (err != ESP_OK || status != 200) {
        ESP_LOGW(TAG, "send failed: %s, HTTP %d", esp_err_to_name(err), status);
        return false;
    }
    last_us = now_us;
    ESP_LOGI(TAG, "sent counts %u, mass %u, total %lu", counts, mass, (unsigned long)daily_total);
    return true;
}
