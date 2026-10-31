// Host-side tests for interpolation.c; see test/Makefile. Expected values are worked out by hand.
#include "interpolation.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    // At or above the top of the table: clamped to the largest mass.
    assert(mass_from_counts(966) == 1000);
    assert(mass_from_counts(1023) == 1000);

    // Exactly on table points.
    assert(mass_from_counts(572) == 400);
    assert(mass_from_counts(33) == 146);
    assert(mass_from_counts(0) == 0);

    // Between points: 284 -> 300 and 572 -> 400, so 428 is 300 + 144 * 100 / 288 = 350.
    assert(mass_from_counts(428) == 350);
    // Bottom segment: 0 -> 0 and 33 -> 146, so 16 is 16 * 146 / 33 = 70 (truncated).
    assert(mass_from_counts(16) == 70);

    puts("interpolation: all tests passed");
    return 0;
}
