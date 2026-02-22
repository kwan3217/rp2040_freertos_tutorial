## Lesson 4 - Memory Allocation
* [Web page](https://www.digikey.com/en/maker/projects/introduction-to-rtos-solution-to-part-4-memory-management/6d4dfcaa1ff84f57a2098da8e6401d9c)
* [Video](https://www.youtube.com/watch?v=Qske3yZRW5I)

### Challenge
> Using FreeRTOS, create two separate tasks. One listens for input over UART (from the
Serial Monitor). Upon receiving a newline character (‘\n’), the task allocates a
new section of heap memory (using pvPortMalloc()) and stores the string up to the
newline character in that section of heap. It then notifies the second task that a
message is ready.
>
> The second task waits for notification from the first task. When it receives that
notification, it prints the message in heap memory to the Serial Monitor. Finally,
it deletes the allocated heap memory (using vPortFree()).

### My Solution
This is before the queue section, so we will (inappropriately) use global variables.
We will carefully use volatile global variables even though queues are more appropriate
(we haven't reached that lesson yet)

[main.c](https://github.com/kwan3217/rp2040_freertos_tutorial/blob/lesson4/main.c)
