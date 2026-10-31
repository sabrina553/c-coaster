// Host-side tests for stats.c; see test/Makefile. Expected values are worked out by hand.
#include "stats.h"
#include <assert.h>
#include <stdio.h>

static void test_median(void)
{
    uint16_t scratch[8];

    const uint16_t one[] = {7};
    assert(median_u16(one, 1, scratch) == 7);

    const uint16_t odd[] = {5, 1, 3};
    assert(median_u16(odd, 3, scratch) == 3);

    // Even count: mean of the two middle values, truncated: (2 + 3) / 2 = 2.
    const uint16_t even[] = {4, 1, 3, 2};
    assert(median_u16(even, 4, scratch) == 2);

    // The 32-bit sum keeps 1023 + 1023 from overflowing.
    const uint16_t top[] = {1023, 1023};
    assert(median_u16(top, 2, scratch) == 1023);

    // The input keeps its order; only scratch is sorted.
    assert(odd[0] == 5 && odd[1] == 1 && odd[2] == 3);
}

static void test_mean(void)
{
    const uint16_t v[] = {100, 100, 100, 100, 100, 100, 101, 101, 102, 102};
    assert(mean_u16(v, 10) == 101); // 1006 / 10 = 100.6 rounds to 101
    assert(mean_u16(v, 6) == 100);
    assert(mean_u16(v, 0) == 0); // empty input doesn't divide by zero
}

static void test_stable_counts(void)
{
    uint16_t scratch[10], out[10];

    // 40% of 10 is the last 4. Median of {100, 100, 200, 100} is 100, and 200 is
    // outside 3%, so 3 are kept.
    const uint16_t in[] = {9, 9, 9, 9, 9, 9, 100, 100, 200, 100};
    size_t kept = stable_counts(in, 10, 40, 3, scratch, out);
    assert(kept == 3);
    assert(out[0] == 100 && out[1] == 100 && out[2] == 100);

    assert(stable_counts(in, 0, 40, 3, scratch, out) == 0);

    // 1% of 10 rounds to 0, which is clamped to 1: just the last sample.
    assert(stable_counts(in, 10, 1, 3, scratch, out) == 1);
    assert(out[0] == 100);
}

int main(void)
{
    test_median();
    test_mean();
    test_stable_counts();
    puts("stats: all tests passed");
    return 0;
}
