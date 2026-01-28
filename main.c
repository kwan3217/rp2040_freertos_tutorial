// main.c
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

// Task parameters
#define BLINK_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1 )
#define PRINT_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1 )
#define BLINK_STACK_SIZE        ( 256 )   // words
#define PRINT_STACK_SIZE        ( 256 )

// Shared LED pin (could be passed as parameter later)
static const uint LED_PIN = PICO_DEFAULT_LED_PIN;

// Blink task: replaces the original while loop
void vBlinkTask(void *pvParameters) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    for (;;) {
        gpio_put(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));   // Tick-aware delay (better than sleep_ms for RTOS)
        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Print task: simulates a "sensor" or logger task
void vPrintTask(void *pvParameters) {
    uint32_t counter = 0;

    for (;;) {
        printf("FreeRTOS Blink count: %u\n", ++counter);
        vTaskDelay(pdMS_TO_TICKS(510));   // Slightly offset so output doesn't align perfectly with LED
    }
}

int main() {
    stdio_init_all();

    printf("Starting FreeRTOS demo...\n");

    // Create blink task
    xTaskCreate(
        vBlinkTask,
        "Blink",                  // task name (for debugging)
        BLINK_STACK_SIZE,
        NULL,                     // parameters (none yet)
        BLINK_TASK_PRIORITY,
        NULL                      // task handle (none needed)
    );

    // Create print task
    xTaskCreate(
        vPrintTask,
        "Print",
        PRINT_STACK_SIZE,
        NULL,
        PRINT_TASK_PRIORITY,
        NULL
    );

    // Start the scheduler — this never returns
    vTaskStartScheduler();

    // Should never reach here
    panic_unsupported();
    return 0;
}
