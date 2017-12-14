#include <libopencm3/cm3/systick.h>
#include "display.h"
#include "../pill.h"

#define DISP_MAX_LEN	100

static volatile bool disp_data[DISP_MAX_LEN];
static volatile uint32_t disp_len = 0;
static volatile bool disp_enabled = false;
static volatile bool disp_once = false;
static volatile bool disp_busy = false;
static volatile uint32_t disp_systick = 0;
static volatile uint32_t disp_value = 0;

uint32_t power(uint32_t base, uint32_t exponent) {
	if (exponent == 0)
		return 1;
	if (exponent == 1)
		return base;
	if (exponent >= 2) {
		int i;
		uint32_t res = base;
		for (i = 2; i <= exponent; i++)
			res *= base;
		return res;
	}
	return 0;
}

uint32_t logarithm(uint32_t base, uint32_t value) {
	uint32_t exponent = 0;
	uint32_t divider = base;
	while (true) {
		if (((double)value / divider) < 1.0)
			return exponent;
		exponent++;
		divider *= base;
	}
}

uint32_t logarithm10(uint32_t value) {
	return logarithm(10, value);
}

void display_clear(void) {
	// Clearing the diplay data array
	disp_enabled = true;
	disp_len = 0;
	int i;
	for (i = 0; i < DISP_MAX_LEN; i++)
		disp_data[i] = false;
}

void display_init(uint32_t value, bool once) {
	// Return if displaying is in progress
	if (disp_busy)
		return;
	disp_busy = true;
	// Clearing the diplay data
	display_clear();
	// Calculating new display values
	int i;
	if (value > 0) {
		uint32_t exponent = logarithm10(value);
		uint32_t whole = (uint32_t)((double)value / power(10, exponent));
		uint32_t decimal = (uint32_t)((((double)value / power(10, exponent)) - whole) * 10.0);
		uint32_t tmp_len = 0;
		for (i = tmp_len; i < whole * 2 + tmp_len; i += 2)
			if (i < DISP_MAX_LEN) 
				disp_data[i] = true;
		tmp_len = i + 2;
		for (i = tmp_len; i < decimal * 2 + tmp_len; i += 2)
			if (i < DISP_MAX_LEN) 
				disp_data[i] = true;
		tmp_len = i + 2;
		for (i = tmp_len; i < exponent * 2 + tmp_len; i += 2)
			if (i < DISP_MAX_LEN) 
				disp_data[i] = true;
		tmp_len = i + 8;
		disp_len = tmp_len;
	}
	else {
		for (i = 0; i < 5; i++)
			disp_data[i] = true;
		disp_len = 12;
	}
	disp_once = once;
	disp_systick = systick_ms;
	disp_enabled = true;
}

void display_cont(uint32_t value) {
	display_init(value, false);
}

void display_once(uint32_t value) {
	display_init(value, true);
}

void display() {
	if (!disp_enabled || disp_len == 0)
		return;
	
	int i = (uint32_t)((systick_ms - disp_systick) / 200);
	if (i >= DISP_MAX_LEN)
		i = DISP_MAX_LEN - 1;
	
	if (disp_data[i])
		LED_ON();
	else
		LED_OFF();
	
	if (i > disp_len) {
		if (!disp_once)
			disp_systick = systick_ms;
		disp_busy = false;
	}
}

void display_enable(void) {
	disp_enabled = true;
}

void display_disable(void) {
	disp_enabled = false;
}
