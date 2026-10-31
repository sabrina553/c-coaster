#include "stats.h"

static int cmp_u16(const void *a, const void *b)
{
    uint16_t x = *(const uint16_t *)a, y = *(const uint16_t *)b;
    return (x > y) - (x < y);
}

// Median of tail[0..n), sorting a copy in scratch so tail keeps its time order.
uint16_t median_u16(const uint16_t *tail, size_t n, uint16_t *scratch)
{
    memcpy(scratch, tail, n * sizeof *tail);
    qsort(scratch, n, sizeof *scratch, cmp_u16);
    if (n % 2) {
        return scratch[n / 2];
    }
    return (uint16_t)(((uint32_t)scratch[n / 2 - 1] + scratch[n / 2]) / 2);
}

// Keeps the last tail_pct % of readings within tol_pct % of their median.
size_t stable_counts(const uint16_t *in, size_t len, unsigned tail_pct, unsigned tol_pct,
                     uint16_t *scratch, uint16_t *out)
{
    if (len == 0) {
        return 0;
    }
    size_t n = (len * tail_pct + 50) / 100; // +50 rounds to nearest
    if (n < 1) {
        n = 1;
    }
    const uint16_t *tail = in + (len - n);
    uint32_t mid = median_u16(tail, n, scratch);

    size_t kept = 0;
    for (size_t i = 0; i < n; i++) {
        uint32_t v = tail[i];
        uint32_t diff = (v > mid) ? v - mid : mid - v;
        if (diff * 100 <= mid * tol_pct) {
            out[kept++] = tail[i];
        }
    }
    return kept;
}

// Integer mean; the sum is 32-bit because 400 * 1023 overflows uint16_t.
uint16_t mean_u16(const uint16_t *v, size_t n)
{
    if (n == 0) {
        return 0;
    }
    uint32_t sum = 0;
    for (size_t i = 0; i < n; i++) {
        sum += v[i];
    }
    return (uint16_t)((sum + n / 2) / n); // + n/2 rounds to nearest
}
