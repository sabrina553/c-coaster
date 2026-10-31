#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

// SNTP wall-clock time in UK local time. Needs Wi-Fi up first.
// Returns true once the clock has synced, false on timeout (SNTP keeps trying).
bool time_sync_init(uint32_t timeout_ms);
bool time_is_synced(void);
// True the first time it sees a local date different from the last call's.
bool time_is_new_day(time_t now);
