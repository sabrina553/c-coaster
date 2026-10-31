#include "interpolation.h"
#include <stddef.h>

uint16_t voltageToMass(uint16_t voltage)
{
    // static const: one copy in flash, not rebuilt on the stack each call.
    static const uint16_t masses[19] = {1000, 950, 900, 850, 800, 750, 700, 650, 600, 550,
                                 500,  450, 400, 350, 300, 250, 200, 150, 0};
    //    static const uint16_t voltages[19] = [
    //    968.9, 967.3, 965.0, 959.7, 953.5, 950.6, 937.6, 933.5, 923.5, 889.3, 868.6, 755.4, 559.3,
    //    133.8, 118.6, 30.6, 23.4, 16.9, 0
    //    ]
    static const uint16_t voltages[19] = {968, 967, 965, 959, 953, 950, 937, 933, 923, 889,
                                   868, 755, 559, 133, 118, 30,  23,  16,  0};
    const size_t n = sizeof voltages / sizeof *voltages;
    if (voltage >= voltages[0]) {
        return masses[0];
    }
    // walk up from - to the first point above the reading v[i] <= voltage < v[i-1]
    // size_t matches n, so no signed/unsigned mix; n >= 1, so n - 1 can't wrap.
    for (size_t i = n - 1; i > 0; i--) {
        if (voltage < voltages[i - 1]) {
            uint32_t vDiff = voltages[i - 1] - voltages[i]; // > 0: table is strictly descending
            uint32_t mDiff = masses[i - 1] - masses[i];
            // Fits uint16_t: the result lies between masses[i] and masses[i - 1].
            return (uint16_t)(masses[i] + (uint32_t)(voltage - voltages[i]) * mDiff / vDiff);
        }
    }
    return 0;
}
