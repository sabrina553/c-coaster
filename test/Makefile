# Host-side unit tests for the hardware-free modules. Run from the repo root: make -C test
# Sanitisers stop on out-of-bounds access and undefined behaviour:
# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wconversion -Werror -g -fsanitize=address,undefined -I../main
OUT     := ../build-host
TESTS   := $(OUT)/test_stats $(OUT)/test_interpolation $(OUT)/test_consumption

.PHONY: all clean
all: $(TESTS)
	@for t in $(TESTS); do $$t || exit 1; done

$(OUT)/test_stats: test_stats.c ../main/stats.c ../main/stats.h | $(OUT)
	$(CC) $(CFLAGS) test_stats.c ../main/stats.c -o $@

$(OUT)/test_interpolation: test_interpolation.c ../main/interpolation.c ../main/interpolation.h | $(OUT)
	$(CC) $(CFLAGS) test_interpolation.c ../main/interpolation.c -o $@

$(OUT)/test_consumption: test_consumption.c ../main/consumption.c ../main/consumption.h | $(OUT)
	$(CC) $(CFLAGS) test_consumption.c ../main/consumption.c -o $@

$(OUT):
	mkdir -p $@

clean:
	rm -rf $(OUT)
