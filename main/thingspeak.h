#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

// Posts one update to ThingSpeak: field1 = mass, field2 = daily total.
// Returns false if rate-limited, offline or the server rejected it.
bool thingspeak_send(uint16_t mass, uint32_t daily_total, time_t timestamp);
