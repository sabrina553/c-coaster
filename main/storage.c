#include "storage.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "storage";
#define NAMESPACE "coaster"

void storage_init(void)
{
    // Erase if the partition is full or was written by a newer NVS format.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

bool storage_load(consumption_t *c)
{
    nvs_handle_t h;
    if (nvs_open(NAMESPACE, NVS_READONLY, &h) != ESP_OK) {
        return false; // namespace doesn't exist until the first save
    }
    consumption_t tmp;
    uint8_t has_last = 0;
    // Load into tmp so a half-read state never reaches the caller.
    bool ok = nvs_get_u16(h, "last_mass", &tmp.last_mass) == ESP_OK &&
              nvs_get_u32(h, "total", &tmp.total) == ESP_OK &&
              nvs_get_i32(h, "day", &tmp.day) == ESP_OK &&
              nvs_get_u8(h, "has_last", &has_last) == ESP_OK;
    nvs_close(h);
    if (!ok) {
        return false;
    }
    tmp.has_last = has_last;
    *c = tmp;
    ESP_LOGI(TAG, "restored last %u g, total %lu g, day %ld", c->last_mass,
             (unsigned long)c->total, (long)c->day);
    return true;
}

void storage_save(const consumption_t *c)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, &h);
    if (err == ESP_OK) {
        nvs_set_u16(h, "last_mass", c->last_mass);
        nvs_set_u32(h, "total", c->total);
        nvs_set_i32(h, "day", c->day);
        nvs_set_u8(h, "has_last", c->has_last);
        err = nvs_commit(h); // the sets are only in RAM until this
        nvs_close(h);
    }
    // A failed save loses at most this reading's state, so log and carry on.
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "save failed: %s", esp_err_to_name(err));
    }
}
