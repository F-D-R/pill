#include <libopencm3/stm32/gpio.h>

#define LED_CLOCK RCC_GPIOC
#define LED_PORT GPIOC
#define LED_PIN GPIO13
#define LED_ON() gpio_clear(LED_PORT, LED_PIN)
#define LED_OFF() gpio_set(LED_PORT, LED_PIN)
#define LED_TOGGLE() gpio_toggle(LED_PORT, LED_PIN)

#define JUMPER_CLOCK RCC_GPIOB
#define JUMPER_PORT GPIOB
#define JUMPER_PIN GPIO2

extern volatile uint32_t systick_ms;
