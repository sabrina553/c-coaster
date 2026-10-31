// Host-side tests for interpolation.c; see test/Makefile. Expected values are worked out by hand.
#include "interpolation.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    // At or above the top of the table: clamped to the largest mass.
    assert(voltageToMass(968) == 1000);
    assert(voltageToMass(1023) == 1000);

    // Exactly on table points.
    assert(voltageToMass(559) == 400);
    assert(voltageToMass(16) == 150);
    assert(voltageToMass(0) == 0);

    // Between points: 868 -> 500 and 889 -> 550, so 878 is 500 + 10 * 50 / 21 = 523 (truncated).
    assert(voltageToMass(878) == 523);
    // Bottom segment: 0 -> 0 and 16 -> 150, so 8 is 8 * 150 / 16 = 75.
    assert(voltageToMass(8) == 75);

    puts("interpolation: all tests passed");
    return 0;
}
