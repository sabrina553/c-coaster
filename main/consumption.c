#include "consumption.h"

void consumption_init(consumption_t *c) { *c = (consumption_t){.day = CONSUMPTION_DAY_UNKNOWN}; }

int32_t consumption_update(consumption_t *c, uint16_t new_mass, int32_t day)
{
    if (day != CONSUMPTION_DAY_UNKNOWN && day != c->day) {
        c->total = 0;
        c->day = day;
    }
    if (!c->has_last) {
        c->last_mass = new_mass;
        c->has_last = true;
        return 0;
    }

    // Positive means it got lighter, i.e. a drink.
    int32_t diff = (int32_t)c->last_mass - (int32_t)new_mass;
    c->last_mass = new_mass;
    if (diff <= -REFILL_THRESHOLD_G) {
        return 0;
    }

    // Small gains subtract on purpose: creep then cancels out, so the total stays
    // "first reading minus latest". Clamp so early creep can't go below zero.
    int64_t total = (int64_t)c->total + diff;
    uint32_t clamped = total < 0 ? 0 : (uint32_t)total;
    int32_t added = (int32_t)((int64_t)clamped - (int64_t)c->total);
    c->total = clamped;
    return added;
}
