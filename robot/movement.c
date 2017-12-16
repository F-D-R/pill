#include <stdint.h>
#include "movement.h"
#include "pwm.h"
#include "../pill.h"

#define MAX_SPEED 1000
#define MIN_SPEED -1000
static volatile int speed = 0;
#define MAX_DIR 180
#define MIN_DIR -180
static volatile int dir = 0;
#define MAX_PWM 2000
#define MIN_PWM 1000
static volatile uint16_t pwm_left = 1500;
static volatile uint16_t pwm_right = 1500;

void calc_output(void) {
	if (speed > MAX_SPEED)
		speed = MAX_SPEED;
	else if (speed < MIN_SPEED)
		speed = MIN_SPEED;
	
	if (dir > MAX_DIR)
		dir = MAX_DIR;
	else if (dir < MIN_DIR)
		dir = MIN_DIR;
	
	pwm_left = (speed / 2) + dir + 500;
	pwm_right = (speed / 2) - dir + 500;
	
	if (pwm_left > MAX_PWM)
		pwm_left = MAX_PWM;
	else if (pwm_left < MIN_PWM)
		pwm_left = MIN_PWM;
	
	if (pwm_right > MAX_PWM)
		pwm_right = MAX_PWM;
	else if (pwm_right < MIN_PWM)
		pwm_right = MIN_PWM;
	
	set_pwm_output(pwm_left, pwm_right);
}

void stop(void) {
	speed = 0;
	dir = 0;
	calc_output();
}

void forward(void) {
	speed = 100;
	dir = 0;
	calc_output();
}

void backward(void) {
	speed = -100;
	dir = 0;
	calc_output();
}

void accelerate(void) {
	speed = (int)(speed * 1.2);
	calc_output();
}

void decelerate(void) {
	speed = (int)(speed * 0.8);
	calc_output();
}

void turn_left(uint16_t degrees) {
	dir = -degrees;
	calc_output();
}

void turn_right(uint16_t degrees){
	dir = degrees;
	calc_output();
}
