#pragma once
#include <stdbool.h>
#include <stdint.h>

// Daily water consumption from one stable mass reading per placement.
// Pure logic (no hardware), so it's tested on the host: test/test_consumption.c

// A gain at least this big is a refill, not sensor creep.
#define REFILL_THRESHOLD_G 10
// Day key meaning "clock not synced yet": the reading counts towards the current day.
#define CONSUMPTION_DAY_UNKNOWN 0

typedef struct {
    uint16_t last_mass; // reference reading in grams
    uint32_t total;     // grams drunk today
    int32_t day;        // YYYYMMDD of total, or CONSUMPTION_DAY_UNKNOWN
    bool has_last;      // false until the first reading ever
} consumption_t;

void consumption_init(consumption_t *c);
// Applies one reading taken on `day` (YYYYMMDD). A new day zeroes the total but
// keeps last_mass, so a cup left overnight still counts from where it was.
// Returns the grams added: negative for small gains (creep), 0 for a refill.
int32_t consumption_update(consumption_t *c, uint16_t new_mass, int32_t day);
