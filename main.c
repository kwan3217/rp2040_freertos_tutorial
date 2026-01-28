// main.c
#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;  // 25 on Pico 1
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    printf("Pico SDK Blinky starting...\n");

    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);

        static uint32_t counter = 0;
        printf("Blink count: %u\n", ++counter);
        sleep_ms(10);  // avoid spamming USB
    }

    return 0;  // unreachable, but good style
}
