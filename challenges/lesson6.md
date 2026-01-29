## Lesson 6 - Mutex
* [Web page]( )
* [Video]( )

### Challenge

> Starting with the code given below, modify it to protect the task parameter
(delay_arg) with a mutex. With the mutex in place, the task should be able
to read the parameter (parameters) into the local variable (num) before the
calling function’s stack memory goes out of scope (the value given by
delay_arg).
>
``` c++
/**
 * FreeRTOS Mutex Challenge
 * 
 * Pass a parameter to a task using a mutex.
 * 
 * Date: January 20, 2021
 * Author: Shawn Hymel
 * License: 0BSD
 */

// You'll likely need this on vanilla FreeRTOS
//#include semphr.h

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Pins (change this if your Arduino board does not have LED_BUILTIN defined)
static const int led_pin = LED_BUILTIN;

//*****************************************************************************
// Tasks

// Blink LED based on rate passed by parameter
void blinkLED(void *parameters) {

  // Copy the parameter into a local variable
  int num = *(int *)parameters;

  // Print the parameter
  Serial.print("Received: ");
  Serial.println(num);

  // Configure the LED pin
  pinMode(led_pin, OUTPUT);

  // Blink forever and ever
  while (1) {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(num / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(num / portTICK_PERIOD_MS);
  }
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  long int delay_arg;

  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Mutex Challenge---");
  Serial.println("Enter a number for delay (milliseconds)");

  // Wait for input from Serial
  while (Serial.available() <= 0);

  // Read integer value
  delay_arg = Serial.parseInt();
  Serial.print("Sending: ");
  Serial.println(delay_arg);

  // Start task 1
  xTaskCreatePinnedToCore(blinkLED,
                          "Blink LED",
                          1024,
                          (void *)&delay_arg,
                          1,
                          NULL,
                          app_cpu);

  // Show that we accomplished our task of passing the stack-based argument
  Serial.println("Done!");
}

void loop() {
  
  // Do nothing but allow yielding to lower-priority tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
```
> When you run the code above as-is, you should see that regardless of what
you enter for the number, the task will always read 0. That’s because the
local variable (delay_arg) goes out of scope before the task can read the
value given by the parameters pointer.
> ![Parameter goes out of scope in Serial output](challenges/images/challenge6A.jpeg)
> After adding a mutex, the task should be able to read the parameters
variable before the memory location goes out of scope. This is what you
should see in the Serial terminal if you did it right:
> ![Parameter stays in scope in Serial output](challenges/images/challenge6B.jpeg)
> **Warning!** This is a hack for two reasons:
> 1. The FreeRTOS documentation strongly discourages using stack memory to
     pass arguments to the task creation process, which is exactly what
     we’re doing here (even if it does work). See the note on `pvParameters`
     in [this API reference for `xTaskCreate`](https://www.freertos.org/a00125.html)
     for more information.
> 2. This is an inappropriate use of a mutex. In fact, it’s closer to how we
     should use a semaphore (to signal to another task that some information
     is ready to be consumed). However, that’s perfect, as we will cover
     semaphores in the next lecture.