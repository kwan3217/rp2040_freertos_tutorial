// main.c
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

// Task parameters
#define BLINK_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1 )
#define PRINT_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1 )
#define READ_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2 )
#define BLINK_STACK_SIZE        ( 256 )   // words
#define PRINT_STACK_SIZE        ( 256 )

// Shared LED pin (could be passed as parameter later)
static const uint LED_PIN = PICO_DEFAULT_LED_PIN;

BaseType_t delay=250;
BaseType_t blink_count=0;

// Blink task: replaces the original while loop
void vBlinkTask(void *pvParameters) {
     
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    for (;;) {
        gpio_put(LED_PIN, 1);
        blink_count++;
        vTaskDelay(pdMS_TO_TICKS(delay));   // Tick-aware delay (better than sleep_ms for RTOS)
        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}

// Print task: simulates a "sensor" or logger task
void vPrintTask(void *pvParameters) {
    for (;;) {
        printf("FreeRTOS Blink count: %u delay: %d\n", blink_count, delay);
        vTaskDelay(pdMS_TO_TICKS(510));   // Slightly offset so output doesn't align perfectly with LED
    }
}

void vReadTask(void *pvParameters) {
  BaseType_t next_delay=0;
  BaseType_t has_input=0;
  for(;;) {
    int c = getchar_timeout_us(0);
    if (c != PICO_ERROR_TIMEOUT) {
        if(c>='0' && c<='9') {
          has_input=1;
          next_delay=next_delay*10+(c-'0');
        } else {
          if(has_input) delay=next_delay;
          next_delay=0;
          has_input=0;
        }
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

int main() {
    stdio_init_all();

    printf("Starting FreeRTOS demo...\n");

    // Create blink task
    xTaskCreate(
        vBlinkTask,
        "BlinkA",                  // task name (for debugging)
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

    // Create print task
    xTaskCreate(
        vReadTask,
        "Read",
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
