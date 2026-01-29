// main.c
#include <string.h>
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

char* volatile msg_ptr=NULL;

// Print task: simulates a "sensor" or logger task
void vPrintTask(void *pvParameters) {
    for (;;) {
      if (msg_ptr) {
        char* this_msg_ptr=msg_ptr;
        printf("%s\n",this_msg_ptr);
        vPortFree(this_msg_ptr);
        msg_ptr=NULL;
      }
      vTaskDelay(pdMS_TO_TICKS(10));   // Slightly offset so output doesn't align perfectly with LED
    }
}

void vReadTask(void *pvParameters) {
  char buf[40];
  int i_buf=0;
  for(;;) {
    int c = getchar_timeout_us(0);
    if (c != PICO_ERROR_TIMEOUT) {
        if(c!=0x0d) {
          buf[i_buf]=c;
          i_buf++;
        } else {
          buf[i_buf]=0;
          i_buf++;
          char* this_msg_ptr=pvPortMalloc(i_buf);
          memcpy(this_msg_ptr,buf,i_buf);
          msg_ptr=this_msg_ptr;
          i_buf=0;
        }
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

int main() {
    stdio_init_all();

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
