// Host-side tests for consumption.c; see test/Makefile. Expected values are worked out by hand.
#include "consumption.h"
#include <assert.h>
#include <stdio.h>

#define DAY1 20261031
#define DAY2 20261101

static void test_first_reading_sets_reference(void)
{
    consumption_t c;
    consumption_init(&c);
    assert(consumption_update(&c, 500, DAY1) == 0);
    assert(c.total == 0 && c.last_mass == 500 && c.has_last);
}

static void test_drinks_add_up(void)
{
    consumption_t c;
    consumption_init(&c);
    consumption_update(&c, 500, DAY1);
    assert(consumption_update(&c, 400, DAY1) == 100);
    assert(consumption_update(&c, 350, DAY1) == 50);
    assert(c.total == 150);
}

static void test_refill_adds_nothing(void)
{
    consumption_t c;
    consumption_init(&c);
    consumption_update(&c, 500, DAY1);
    consumption_update(&c, 200, DAY1);              // drank 300
    assert(consumption_update(&c, 600, DAY1) == 0); // refill
    assert(c.total == 300 && c.last_mass == 600);
    assert(consumption_update(&c, 500, DAY1) == 100); // counted from the refill
    assert(c.total == 400);

    // Exactly at the threshold is a refill; just under is creep.
    consumption_update(&c, 500 + REFILL_THRESHOLD_G, DAY1);
    assert(c.total == 400);
    assert(consumption_update(&c, 510 + REFILL_THRESHOLD_G - 1, DAY1) == -(REFILL_THRESHOLD_G - 1));
}

// The worked example from Docs/Placement Readings: creep up then back cancels out.
static void test_creep_cancels(void)
{
    consumption_t c;
    consumption_init(&c);
    consumption_update(&c, 500, DAY1);
    consumption_update(&c, 400, DAY1);               // drink 100
    assert(consumption_update(&c, 405, DAY1) == -5); // creep
    consumption_update(&c, 305, DAY1);               // drink 100
    consumption_update(&c, 200, DAY1);               // drink 100, creep gone
    assert(c.total == 300);
}

static void test_total_never_negative(void)
{
    consumption_t c;
    consumption_init(&c);
    consumption_update(&c, 500, DAY1);
    assert(consumption_update(&c, 505, DAY1) == 0); // clamped: nothing to subtract from
    assert(c.total == 0);
    consumption_update(&c, 495, DAY1);
    assert(c.total == 10);
}

static void test_new_day_resets_total_not_reference(void)
{
    consumption_t c;
    consumption_init(&c);
    consumption_update(&c, 500, DAY1);
    consumption_update(&c, 300, DAY1);
    assert(c.total == 200);
    // Cup left overnight: tomorrow's first drink counts from 300.
    assert(consumption_update(&c, 250, DAY2) == 50);
    assert(c.total == 50 && c.day == DAY2);
}

static void test_unknown_day_keeps_current_day(void)
{
    consumption_t c;
    consumption_init(&c);
    consumption_update(&c, 500, DAY1);
    consumption_update(&c, 400, CONSUMPTION_DAY_UNKNOWN); // clock lost after a reboot
    assert(c.total == 100 && c.day == DAY1);
}

int main(void)
{
    test_first_reading_sets_reference();
    test_drinks_add_up();
    test_refill_adds_nothing();
    test_creep_cancels();
    test_total_never_negative();
    test_new_day_resets_total_not_reference();
    test_unknown_day_keeps_current_day();
    puts("consumption: all tests passed");
    return 0;
}
