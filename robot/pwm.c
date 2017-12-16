#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include "../pill.h"
#include "pwm.h"

#define PWM_OUTPUT_TIMER_CLOCK	RCC_TIM3
#define PWM_OUTPUT_TIMER		TIM3
#define PWM_OUTPUT_LEFT_OC		TIM_OC1
#define PWM_OUTPUT_RIGHT_OC		TIM_OC2
#define PWM_OUTPUT_PORT_CLOCK	RCC_GPIOA
#define PWM_OUTPUT_PORT			GPIOA
#define PWM_OUTPUT_LEFT_PIN		GPIO_TIM3_CH1
#define PWM_OUTPUT_RIGHT_PIN	GPIO_TIM3_CH2

void setup_pwm_output(void) {
	rcc_periph_clock_enable(PWM_OUTPUT_PORT_CLOCK);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(PWM_OUTPUT_TIMER_CLOCK);
	
	gpio_set_mode(PWM_OUTPUT_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, PWM_OUTPUT_LEFT_PIN | PWM_OUTPUT_RIGHT_PIN);
	
	timer_disable_counter(PWM_OUTPUT_TIMER);
	
	timer_direction_up(PWM_OUTPUT_TIMER);
	timer_continuous_mode(PWM_OUTPUT_TIMER);
	timer_set_prescaler(PWM_OUTPUT_TIMER, 71);
	timer_set_period(PWM_OUTPUT_TIMER, 20000);
	
	timer_set_oc_mode(PWM_OUTPUT_TIMER, PWM_OUTPUT_LEFT_OC, TIM_OCM_PWM1);
	timer_set_oc_mode(PWM_OUTPUT_TIMER, PWM_OUTPUT_RIGHT_OC, TIM_OCM_PWM1);
	
	timer_enable_oc_preload(PWM_OUTPUT_TIMER, PWM_OUTPUT_LEFT_OC);
	timer_enable_oc_preload(PWM_OUTPUT_TIMER, PWM_OUTPUT_RIGHT_OC);
	
	timer_set_oc_value(PWM_OUTPUT_TIMER, PWM_OUTPUT_LEFT_OC, 1500);
	timer_set_oc_value(PWM_OUTPUT_TIMER, PWM_OUTPUT_RIGHT_OC, 1500);
	
	timer_enable_oc_output(PWM_OUTPUT_TIMER, PWM_OUTPUT_LEFT_OC);
	timer_enable_oc_output(PWM_OUTPUT_TIMER, PWM_OUTPUT_RIGHT_OC);
	
	timer_enable_counter(PWM_OUTPUT_TIMER);
}

void set_pwm_output(uint16_t left, uint16_t right) {
	timer_set_oc_value(PWM_OUTPUT_TIMER, PWM_OUTPUT_LEFT_OC, left);
	timer_set_oc_value(PWM_OUTPUT_TIMER, PWM_OUTPUT_RIGHT_OC, right);
}
