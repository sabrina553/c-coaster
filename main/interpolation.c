#include "interpolation.h"
#include <stddef.h>

uint16_t mass_from_counts(uint16_t counts)
{
    // Calibration for this board (2026-10-31): mean ADC counts for known masses.
    // Counts fall as mass falls. Replaces the original SmartCoaster's 19-point table
    // (https://github.com/sabrina553/SmartCoaster).
    // static const: one copy in flash, not rebuilt on the stack each call.
    static const uint16_t masses[] = {1000, 900, 800, 700, 600, 500, 400, 300, 200, 146, 0};
    static const uint16_t table[] = {966, 958, 950, 934, 913, 812, 572, 284, 96, 33, 0};
    _Static_assert(sizeof masses == sizeof table, "calibration tables differ in length");
    const size_t n = sizeof table / sizeof *table;

    if (counts >= table[0]) {
        return masses[0];
    }
    // Walk up from the bottom to the segment table[i] <= counts < table[i - 1].
    // size_t matches n, so no signed/unsigned mix; n >= 1, so n - 1 can't wrap.
    for (size_t i = n - 1; i > 0; i--) {
        if (counts < table[i - 1]) {
            uint32_t count_span = table[i - 1] - table[i]; // > 0: table is strictly descending
            uint32_t mass_span = masses[i - 1] - masses[i];
            // Fits uint16_t: the result lies between masses[i] and masses[i - 1].
            return (uint16_t)(masses[i] + (uint32_t)(counts - table[i]) * mass_span / count_span);
        }
    }
    return 0;
}
