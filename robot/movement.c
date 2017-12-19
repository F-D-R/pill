#include <stdint.h>
#include "movement.h"
#include "pwm.h"
#include "../pill.h"

#define _RND_BOOL() (systick_ms % 2 == 0)

enum MovementType {
	Speed = 1,
	Accelerate = 2,
	Decelerate = 3,
	Angle = 4,
};

struct Movement {
	uint32_t timestamp;
	uint16_t timespan;
	enum MovementType type;
	int16_t value;
	bool started;
	int16_t value_from;
	int16_t value_to;
};

#define MOVEMENTS_MAX_LEN 5
static struct Movement movements[MOVEMENTS_MAX_LEN];
static volatile uint8_t movements_len = 0;

static volatile bool movement_in_progress = false;

#define MAX_SPEED 1000
#define MIN_SPEED -1000
static volatile int16_t speed = 0;

#define MAX_ANGLE 180
#define MIN_ANGLE -180
static volatile int16_t angle = 0;

#define MAX_PWM 2000
#define MIN_PWM 1000
static volatile uint16_t pwm_left = 1500;
static volatile uint16_t pwm_right = 1500;

void clear_movements(void) {
	// Clearing the movements array
	movements_len = 0;
	int i;
	for (i = 0; i < MOVEMENTS_MAX_LEN; i++) {
		movements[i].timestamp = 0;
		movements[i].timespan = 0;
		movements[i].type = 0;
		movements[i].value = 0;
		movements[i].started = false;
		movements[i].value_from = 0;
		movements[i].value_to = 0;
	}
}

void set_movement(uint8_t index, uint32_t timestamp, uint16_t timespan, enum MovementType type, int16_t value) {
	movements[index].timestamp = timestamp;
	movements[index].timespan = timespan;
	movements[index].type = type;
	movements[index].value = value;
}

void add_movement(uint16_t timespan, enum MovementType type, int16_t value) {
	set_movement(
		movements_len, 
		movements_len ? movements[movements_len - 1].timestamp + movements[movements_len - 1].timespan : systick_ms, 
		timespan, 
		type, 
		value
	);
	movements_len++;
}

void calc_output(void) {
	if (speed > MAX_SPEED)
		speed = MAX_SPEED;
	else if (speed < MIN_SPEED)
		speed = MIN_SPEED;
	
	if (angle > MAX_ANGLE)
		angle = MAX_ANGLE;
	else if (angle < MIN_ANGLE)
		angle = MIN_ANGLE;
	
	pwm_left = (speed / 2) + angle + 500;
	pwm_right = (speed / 2) - angle + 500;
	
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
	if (movement_in_progress)
		return;
	movement_in_progress = true;
	clear_movements();
	
	add_movement(500, Speed, 0);
	add_movement(1, Angle, 0);
	calc_output();
}

void stop_and_rnd_turn(uint16_t degrees) {
	if (movement_in_progress)
		return;
	movement_in_progress = true;
	clear_movements();
	
	add_movement(1, Angle, 0);
	add_movement(500, Speed, 0);
	add_movement(500, Angle, _RND_BOOL() ? degrees : -degrees);
	calc_output();
}

void stop_and_rnd_turn_and_forward(uint16_t degrees) {
	if (movement_in_progress)
		return;
	movement_in_progress = true;
	clear_movements();
	
	add_movement(1, Angle, 0);
	add_movement(500, Speed, 0);
	add_movement(500, Angle, _RND_BOOL() ? degrees : -degrees);
	add_movement(500, Speed, 100);
	calc_output();
}

void forward(void) {
	if (movement_in_progress)
		return;
	movement_in_progress = true;
	clear_movements();
	
	add_movement(1, Angle, 0);
	add_movement(500, Speed, 100);
	calc_output();
}

void backward(void) {
	if (movement_in_progress)
		return;
	movement_in_progress = true;
	clear_movements();
	
	add_movement(1, Angle, 0);
	add_movement(500, Speed, -100);
	calc_output();
}

void accelerate(void) {
	if (movement_in_progress)
		return;
	movement_in_progress = true;
	clear_movements();
	
	add_movement(500, Accelerate, 20);
	calc_output();
}

void decelerate(void) {
	if (movement_in_progress)
		return;
	movement_in_progress = true;
	clear_movements();
	
	add_movement(500, Decelerate, 20);
	calc_output();
}

void decelerate_and_rnd_turn(uint16_t degrees) {
	if (movement_in_progress)
		return;
	movement_in_progress = true;
	clear_movements();
	
	add_movement(500, Decelerate, 20);
	add_movement(500, Angle, _RND_BOOL() ? degrees : -degrees);
	calc_output();
}

void turn_left(uint16_t degrees) {
	if (movement_in_progress)
		return;
	movement_in_progress = true;
	clear_movements();
	
	add_movement(500, Angle, -degrees);
	calc_output();
}

void turn_right(uint16_t degrees){
	if (movement_in_progress)
		return;
	movement_in_progress = true;
	clear_movements();
	
	add_movement(500, Angle, degrees);
	calc_output();
}

void rnd_turn(uint16_t degrees) {
	if (movement_in_progress)
		return;
	movement_in_progress = true;
	clear_movements();
	
	add_movement(500, Angle, _RND_BOOL() ? degrees : -degrees);
	calc_output();
}

void calc_movement(void) {
	// Find the current movement based on timestamp
	int i;
	uint32_t current_time = systick_ms;
	for (i = 0; i < movements_len; i++) {
		if (current_time > movements[i].timestamp && current_time < movements[i].timestamp + movements[i].timespan)
			break;
	}
	
	// If no movement has found, it returns
	if (i >= movements_len) {
		movement_in_progress = false;
		return;
	}
	
	bool started = movements[i].started;
	if (!started) {
		// Remember that it is started
		movements[i].started = true;
		// Calculate the from and to values
		switch (movements[i].type) {
			case Speed:
				movements[i].value_from = speed;
				movements[i].value_to = movements[i].value;
				break;
			case Accelerate:
				movements[i].value_from = speed;
				movements[i].value_to = speed + speed * movements[i].value / 100;
				break;
			case Decelerate:
				movements[i].value_from = speed;
				movements[i].value_to = speed - speed * movements[i].value / 100;
				break;
			case Angle:
				movements[i].value_from = angle;
				movements[i].value_to = movements[i].value;
				break;
		}
	}
	else {
		int16_t whole_change = movements[i].value_to - movements[i].value_from;
		uint16_t ellapsed = current_time - movements[i].timestamp;
		
		// Calculate the new value
		switch (movements[i].type) {
			case Speed:
			case Accelerate:
			case Decelerate:
				speed += whole_change * ellapsed / movements[i].timespan;
				break;
			case Angle:
				angle += whole_change * ellapsed / movements[i].timespan;
				break;
		}
	}
}
