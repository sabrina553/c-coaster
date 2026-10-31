#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

// SNTP wall-clock time in UK local time. Needs Wi-Fi up first.
// Returns true once the clock has synced, false on timeout (SNTP keeps trying).
bool time_sync_init(uint32_t timeout_ms);
bool time_is_synced(void);
// Local date as YYYYMMDD (e.g. 20261031), the day key consumption_update() takes.
int32_t time_day_key(time_t now);
