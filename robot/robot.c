#include <libopencm3/stm32/rcc.h>
//#include <libopencm3/stm32/gpio.h>
//#include <libopencm3/stm32/timer.h>
//#include <libopencm3/cm3/nvic.h>
//#include <libopencm3/stm32/exti.h>
//#include <libopencmsis/core_cm3.h>
#include <libopencm3/cm3/systick.h>
#include "display.h"
#include "pwm.h"
#include "distance.h"
#include "../pill.h"

#define _NOP() asm volatile ("nop")

volatile uint32_t systick_ms = 0;

static volatile uint16_t pwm_left = 1000;
static volatile uint16_t pwm_right = 2000;
static volatile uint32_t distance = 0;

void systick_ms_setup(void) {
	/* 72MHz clock, interrupt for every 72,000 CLKs (1ms). */
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_set_reload(72000 - 1);	// Period = 1ms
	systick_clear();
	systick_interrupt_enable();
	systick_counter_enable();
}

int main(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();	// Clock setup
	rcc_periph_clock_enable(LED_CLOCK);			// For LED

	// Setup the LED.
	gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
	LED_ON();

	systick_ms_setup();
	
	while (systick_ms < 3000);
	LED_OFF();
	display_clear();
	display_enable();
	while (systick_ms < 4000);
	
	setup_distance_measurements();
	setup_pwm_output();
	
	while (1) {
		distance = get_distance();
		display_once(distance);
		
		if (distance < 10) {
			
		}
		else if (distance < 20) {
			
		}
		else if (distance < 50) {
			
		}
		else if (distance < 100) {
			
		}
		else {
			
		}
		
		set_speed(pwm_left, pwm_right);
		
		//__WFI();
		_NOP();
	}
	return 0;
}

/* Interrupts */

void sys_tick_handler(void) {
	systick_ms ++;

	// Start distance measuring with an 1ms pulse every 100ms
	if (systick_ms % 100 == 0)
		distance_set_trigger();
	else
		distance_clear_trigger();
	
	// Display a result in normal form by blinking the LED
	display();
	
#if 0
	static bool up = false;
	if (systick_ms % 500 == 0) {
		if (up) {
			pwm_left += 100;
			if (pwm_left >= 2200)
				up = false;
		}
		else {
			pwm_left -= 100;
			if (pwm_left <= 800)
				up = true;
		}
		set_speed(pwm_left, pwm_right);
	}
#endif
}
