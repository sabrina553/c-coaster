// Host-side tests for interpolation.c; see test/Makefile. Expected values are worked out by hand.
#include "interpolation.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    // At or above the top of the table: clamped to the largest mass.
    assert(mass_from_counts(968) == 1000);
    assert(mass_from_counts(1023) == 1000);

    // Exactly on table points.
    assert(mass_from_counts(559) == 400);
    assert(mass_from_counts(16) == 150);
    assert(mass_from_counts(0) == 0);

    // Between points: 868 -> 500 and 889 -> 550, so 878 is 500 + 10 * 50 / 21 = 523 (truncated).
    assert(mass_from_counts(878) == 523);
    // Bottom segment: 0 -> 0 and 16 -> 150, so 8 is 8 * 150 / 16 = 75.
    assert(mass_from_counts(8) == 75);

    puts("interpolation: all tests passed");
    return 0;
}
