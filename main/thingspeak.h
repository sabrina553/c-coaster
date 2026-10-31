#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

// Posts one update using the original SmartCoaster channel layout:
// field1 = ADC counts, field2 = mass (g), field3 = daily total (g).
// timestamp 0 means "clock not synced": ThingSpeak then uses its own receive time.
// Returns false if rate-limited, offline or the server rejected it.
bool thingspeak_send(uint16_t counts, uint16_t mass, uint32_t daily_total, time_t timestamp);
