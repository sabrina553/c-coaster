#pragma once
#include <stdint.h>

// Converts mean ADC counts (0..1023) to grams by linear interpolation of the
// calibration table; clamps above the top point.
uint16_t mass_from_counts(uint16_t counts);
