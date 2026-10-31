#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

uint16_t median_u16(const uint16_t *tail, size_t n, uint16_t *scratch);
size_t stable_counts(const uint16_t *in, size_t len, unsigned tail_pct, unsigned tol_pct,
                     uint16_t *scratch, uint16_t *out);
uint16_t mean_u16(const uint16_t *tail, size_t n);
