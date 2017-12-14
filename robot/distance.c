#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
//#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencmsis/core_cm3.h>
#include "distance.h"
#include "display.h"
#include "../pill.h"

// MODE: INT | IC | IC+DMA
#define MODE				IC

#define DIST_PORT_CLOCK		RCC_GPIOA
#define DIST_PORT			GPIOA
#define DIST_TRG_PIN		GPIO3
#define DIST_ECHO_PIN		GPIO0
#define DIST_TIMER_CLOCK	RCC_TIM2
#define DIST_TIMER			TIM2

static volatile uint32_t distance = 0;
static volatile uint32_t dist_pulse_count = 0;
static volatile uint32_t dist_period_count = 0;

#if MODE == INT
#define FALLING	0
#define RISING	1
uint16_t exti_direction = FALLING;
#endif

void distance_set_trigger(void) {
	gpio_set(DIST_PORT, DIST_TRG_PIN);
}

void distance_clear_trigger(void) {
	gpio_clear(DIST_PORT, DIST_TRG_PIN);
}

void setup_distance_measurements(void) {
	rcc_periph_clock_enable(DIST_PORT_CLOCK);
	gpio_set_mode(DIST_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, DIST_TRG_PIN);
	gpio_set_mode(DIST_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, DIST_ECHO_PIN);
	
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(DIST_TIMER_CLOCK);
	timer_disable_counter(DIST_TIMER);
	
	timer_set_prescaler(DIST_TIMER, 71);	// Period = 1µs
	
#if MODE == INT
	nvic_enable_irq(NVIC_EXTI0_IRQ);
	exti_select_source(EXTI0, DIST_PORT);
	exti_direction = FALLING;
	exti_set_trigger(EXTI0, EXTI_TRIGGER_FALLING);
	exti_enable_request(EXTI0);
#endif

#if MODE == IC
	timer_direction_up(DIST_TIMER);
	timer_continuous_mode(DIST_TIMER);
	// Select the active input for TIMx_CCR1: write the CC1S bits to 01 in the TIMx_CCMR1 register (TI1 selected).
	timer_ic_set_input(DIST_TIMER, TIM_IC1, TIM_IC_IN_TI1);
	// Select the active polarity for TI1FP1 (used both for capture in TIMx_CCR1 and counter clear): write the CC1P to ‘0’  (active on rising edge).
	timer_ic_set_polarity(DIST_TIMER, TIM_IC1, TIM_IC_RISING);
	// Select the active input for TIMx_CCR2: write the CC2S bits to 10 in the TIMx_CCMR1 register (TI1 selected).
	timer_ic_set_input(DIST_TIMER, TIM_IC2, TIM_IC_IN_TI1);
	// Select the active polarity for TI1FP2 (used for capture in TIMx_CCR2): write the CC2P bit to ‘1’ (active on falling edge).
	timer_ic_set_polarity(DIST_TIMER, TIM_IC2, TIM_IC_FALLING);
	// Select the valid trigger input: write the TS bits to 101 in the TIMx_SMCR register (TI1FP1 selected). 
	timer_slave_set_trigger(DIST_TIMER, TIM_SMCR_TS_IT1FP1);
	// Configure the slave mode controller in reset mode: write the SMS bits to 100 in the TIMx_SMCR register. 
	timer_slave_set_mode(DIST_TIMER, TIM_SMCR_SMS_RM);	// Reset Mode - Rising edge of the selected trigger input (TRGI) reinitializes the counter and generates an update of the registers.
	// Enable timer Interrupt(s).
	nvic_set_priority(NVIC_TIM2_IRQ, 2);
	nvic_enable_irq(NVIC_TIM2_IRQ);
	// Enable the Capture/Compare and Update interrupt requests.
	//timer_enable_irq(DIST_TIMER, (TIM_DIER_CC1IE | TIM_DIER_CC2IE | TIM_DIER_UIE));
	timer_enable_irq(DIST_TIMER, TIM_DIER_CC2IE);
	// Enable the captures: write the CC1E and CC2E bits to ‘1 in the TIMx_CCER register.
	timer_ic_enable(DIST_TIMER, TIM_IC1);
	timer_ic_enable(DIST_TIMER, TIM_IC2);
	timer_enable_counter(DIST_TIMER);
#endif

#if MODE == IC+DMA
	
#endif
}

#if MODE == INT
void exti0_isr(void) {
	exti_reset_request(EXTI0);
	
	if (exti_direction == FALLING) {
		display_once(timer_get_counter(DIST_TIMER));
		timer_disable_counter(DIST_TIMER);
		timer_set_counter(DIST_TIMER, 0);
		exti_direction = RISING;
		exti_set_trigger(EXTI0, EXTI_TRIGGER_RISING);
	} 
	else {
		timer_enable_counter(DIST_TIMER);
		exti_direction = FALLING;
		exti_set_trigger(EXTI0, EXTI_TRIGGER_FALLING);
	}
}
#endif

#if MODE == IC
void tim2_isr(void) {
	if (timer_get_flag(DIST_TIMER, TIM_SR_CC1IF)) {	// CC1IF: Capture/compare 1 interrupt flag
		timer_clear_flag(DIST_TIMER, TIM_SR_CC1IF);

		//display_once(timer_get_counter(DIST_TIMER));
		//display_once(TIM2_CCR1);
	}
	if (timer_get_flag(DIST_TIMER, TIM_SR_CC2IF)) {
		timer_clear_flag(DIST_TIMER, TIM_SR_CC2IF);

		//display_once(timer_get_counter(DIST_TIMER));
		display_once(TIM2_CCR2);
	}
	if (timer_get_flag(DIST_TIMER, TIM_SR_UIF)) {
		timer_clear_flag(DIST_TIMER, TIM_SR_UIF);
		// FIXME clear overflow interrupt but what else ?
	}
}
#endif
