#include <stdint.h>
#include "movement.h"
#include "pwm.h"
#include "../pill.h"


void stop(void) {
	set_speed(1500, 1500);
}

void forward(void) {
}

void backward(void) {
}

void accelerate(void) {
}

void decelerate(void) {
}

void turn_left(uint16_t degrees) {
}

void turn_right(uint16_t degrees){
}
