// main.c
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// Task parameters
#define BLINK_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1 )
#define PRINT_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1 )
#define READ_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2 )
#define BLINK_STACK_SIZE        ( 256 )   // words
#define PRINT_STACK_SIZE        ( 256 )

// Shared LED pin (could be passed as parameter later)
static const uint LED_PIN = PICO_DEFAULT_LED_PIN;

QueueHandle_t queue1, queue2;

void vTaskA(void *pvParameters) {
  /*
    Task A should print any new messages it receives from Queue 2. Additionally, it
    should read any Serial input from the user and echo back this input to the serial
    input. If the user enters “delay” followed by a space and a number, it should
    send that number to Queue 1.
  */
  // This could be split into two tasks -- one that prints things from queue 2, and one
  // that watches stdin and stuffs numnbers into queue 1 when needed. we will use the
  // exact design the challenge specifies.
  // we will use a state machine for interpreting the delay command
    char delay_prefix[]="delay ";
    BaseType_t state=0; // 0 - stepping through delay prefix
                        // 1 - expecting digits
    BaseType_t i_prefix=0;
    BaseType_t new_delay=0;
    for (;;) {
      // check queue 2 for messages. if there is one, pull it and print it
      char* msg;
      if (xQueueReceive(queue2,&msg,0)) {
        printf("%s\n",msg);
        vPortFree(msg);
      }
      //check if stdin has any chars
      int c = getchar_timeout_us(0);
      if (c != PICO_ERROR_TIMEOUT) {
        putchar(c);
        switch (state) {
          case 0:
            // stepping through the prefix
            if (c==delay_prefix[i_prefix]) {
              i_prefix++;
            } else {
              i_prefix=0;
            }
            if (i_prefix==sizeof(delay_prefix)-1) {
              state=1;
              i_prefix=0; //ready for next command
              new_delay=0;
            }
            break;
          case 1:
            if (c>='0' && c<='9') {
              new_delay=new_delay*10+(c-'0');
            } else {
              if (c==0x0d) {
                // finished the number, send it
                xQueueSend(queue1,&new_delay,portMAX_DELAY);
              }
              // if here, some bad char in number, start over
              state=0; //ready for next number
            }
            break;
        }
      }
    }
}

void vTaskB(void *pvParameters) {
  /* Task B should read any messages from Queue 1. If it contains a number, it should
    update its delay rate to that number (milliseconds). It should also blink an LED
    at a rate specified by that delay. Additionally, every time the LED blinks 100
    times, it should send the string “Blinked” to Queue 2. You can also optionally send
    the number of times the LED blinked (e.g. 100) as part of struct that encapsulates
    the string and this number.
  */
  BaseType_t new_delay;
  BaseType_t blinks=0;
  BaseType_t delay=100;
  for(;;) {
    // check queue 1 for any entries by non-blocking read. if there is an entry, use it.
    if (xQueueReceive(queue1,&new_delay,0)) {
      delay=new_delay;
    }
    //count blinks
    blinks++;
    if (blinks%100==0) {
      char* buf=pvPortMalloc(32);
      snprintf(buf,32,"Blinked: %d",blinks);
      xQueueSend(queue2,&buf,portMAX_DELAY); //block forever until queue has space, then write pointer to string
    }
    gpio_put(LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(delay));   // Tick-aware delay (better than sleep_ms for RTOS)
    gpio_put(LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(delay));
  }
}

int main() {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    //create queues
    queue1=xQueueCreate(10,sizeof(BaseType_t));
    queue2=xQueueCreate(10,sizeof(char*));

    // Create print task
    xTaskCreate(
        vTaskA,
        "TaskA",
        PRINT_STACK_SIZE,
        NULL,
        PRINT_TASK_PRIORITY,
        NULL
    );

    // Create print task
    xTaskCreate(
        vTaskB,
        "TaskB",
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
