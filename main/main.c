#include "consumption.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "interpolation.h"
#include "mcp3008.h"
#include "stats.h"
#include "storage.h"
#include "thingspeak.h"
#include "time_sync.h"
#include "wifi.h"
#include <time.h>

static const char *TAG = "c-coaster";

#define N_SAMPLES 1000
// Readings at or below this mean nothing is on the coaster.
#define PRESENT_THRESHOLD 10
#define SAMPLE_PERIOD_MS 10
// stable_counts(): use the last 40% of a batch, keeping readings within 3% of its median.
#define STABLE_TAIL_PCT 40
#define STABLE_TOL_PCT 3
// Once recorded, only a lift matters, so poll more slowly.
#define RECORDED_POLL_MS 100
// About 15 minutes of readings at one per placement every ~100 s.
#define QUEUE_LEN 10
// TLS handshakes need ~5-6 kB of stack on their own.
#define NETWORK_STACK 8192
// ThingSpeak's free tier allows one update per 15 s.
#define SEND_GAP_MS 15000

// One placement's result, passed from the sampling loop to the network task.
typedef struct {
    uint16_t counts;
    uint16_t mass;
    uint32_t total;
    time_t timestamp; // 0 if the clock wasn't synced
} reading_t;

// One reading per placement, like the original SmartCoaster: sample until a stable
// mass is recorded, then wait for the cup to be lifted before sampling again.
typedef enum {
    COASTER_EMPTY,    // nothing on the coaster
    COASTER_SAMPLING, // filling the sample buffer
    COASTER_RECORDED, // reading taken; waiting for a lift
} coaster_state_t;

static QueueHandle_t s_readings;

// Sends readings so a slow TLS request never stalls sampling.
static void network_task(void *arg)
{
    (void)arg;
    reading_t r;
    for (;;) {
        xQueueReceive(s_readings, &r, portMAX_DELAY);
        while (!wifi_wait_connected(60000)) {
            ESP_LOGW(TAG, "waiting for Wi-Fi, %u readings queued",
                     (unsigned)uxQueueMessagesWaiting(s_readings));
        }
        if (!thingspeak_send(r.counts, r.mass, r.total, r.timestamp)) {
            ESP_LOGW(TAG, "reading dropped");
        }
        vTaskDelay(pdMS_TO_TICKS(SEND_GAP_MS));
    }
}

static void queue_reading(const reading_t *r)
{
    // Full means Wi-Fi has been down a while: drop the oldest, keep the newest.
    if (xQueueSend(s_readings, r, 0) != pdTRUE) {
        reading_t oldest;
        xQueueReceive(s_readings, &oldest, 0);
        xQueueSend(s_readings, r, 0);
        ESP_LOGW(TAG, "queue full, dropped oldest reading");
    }
}

// Turns a full sample buffer into a recorded reading; false if no stable readings.
static bool record(const uint16_t *samples, uint16_t *scratch, uint16_t *stable, consumption_t *c)
{
    size_t n_stable =
        stable_counts(samples, N_SAMPLES, STABLE_TAIL_PCT, STABLE_TOL_PCT, scratch, stable);
    if (n_stable == 0) {
        ESP_LOGW(TAG, "no stable readings in batch");
        return false;
    }
    uint16_t counts = mean_u16(stable, n_stable);
    uint16_t mass = mass_from_counts(counts);

    bool synced = time_is_synced();
    time_t now = synced ? time(NULL) : 0;
    int32_t day = synced ? time_day_key(now) : CONSUMPTION_DAY_UNKNOWN;
    int32_t added = consumption_update(c, mass, day);
    storage_save(c);

    ESP_LOGI(TAG, "counts %u, mass %u g, %+ld g, total %lu g", counts, mass, (long)added,
             (unsigned long)c->total);
    reading_t r = {.counts = counts, .mass = mass, .total = c->total, .timestamp = now};
    queue_reading(&r);
    return true;
}

void app_main(void)
{
    storage_init();
    static consumption_t consumption;
    if (!storage_load(&consumption)) {
        consumption_init(&consumption);
    }

    mcp3008_init();

    // Sampling works offline, so a slow network only delays the clock, not readings.
    wifi_init();
    bool online = wifi_wait_connected(15000);
    if (!time_sync_init(online ? 10000 : 1)) {
        ESP_LOGW(TAG, "clock not synced yet; readings are untimed until it is");
    }

    s_readings = xQueueCreate(QUEUE_LEN, sizeof(reading_t));
    xTaskCreate(network_task, "network", NETWORK_STACK, NULL, 5, NULL);

    // static keeps these 6 kB off the main task's stack.
    static uint16_t samples[N_SAMPLES];
    static uint16_t scratch[N_SAMPLES];
    static uint16_t stable[N_SAMPLES];
    coaster_state_t state = COASTER_EMPTY;
    size_t i = 0;

    for (;;) {
        uint16_t raw = mcp3008_read(0);
        bool present = raw > PRESENT_THRESHOLD;

        switch (state) {
        case COASTER_EMPTY:
            if (present) {
                ESP_LOGI(TAG, "cup placed");
                i = 0;
                samples[i++] = raw;
                state = COASTER_SAMPLING;
            }
            break;

        case COASTER_SAMPLING:
            if (!present) {
                ESP_LOGI(TAG, "lifted before a reading");
                state = COASTER_EMPTY;
                break;
            }
            samples[i++] = raw;
            if (i == N_SAMPLES) {
                // Unstable batch: sample again rather than give up on this placement.
                i = 0;
                if (record(samples, scratch, stable, &consumption)) {
                    state = COASTER_RECORDED;
                }
            }
            break;

        case COASTER_RECORDED:
            if (!present) {
                ESP_LOGI(TAG, "cup lifted");
                state = COASTER_EMPTY;
            }
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(state == COASTER_RECORDED ? RECORDED_POLL_MS : SAMPLE_PERIOD_MS));
    }
}
