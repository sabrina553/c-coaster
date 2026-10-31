#pragma once
#include "consumption.h"
#include <stdbool.h>

// Keeps the consumption state in NVS (flash key-value store) across reboots,
// like the original's etc/water_consumption_data file.
// https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/storage/nvs_flash.html

// Initialises NVS. Call before wifi_init(), which also stores data in NVS.
void storage_init(void);
// Fills c from flash; false (and c untouched) if nothing has been saved yet.
bool storage_load(consumption_t *c);
// Writes c to flash. Call once per reading, not in a fast loop: flash wears out.
void storage_save(const consumption_t *c);
