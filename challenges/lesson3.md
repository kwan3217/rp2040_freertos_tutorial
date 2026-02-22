## Lesson 3 - non-blocking USB input
* [Web page](https://www.digikey.com/en/maker/projects/introduction-to-rtos-solution-to-part-3-task-scheduling/8fbb9e0b0eed4279a2dd698f02ce125f)
* [Video](https://www.youtube.com/watch?v=95yUbClyf3E)

### Challenge

> Using FreeRTOS, create two separate tasks. One listens for an integer over UART
(from the Serial Monitor) and sets a variable when it sees an integer. The other
task blinks the onboard LED (or other connected LED) at a rate specified by that
integer. In effect, you want to create a multi-threaded system that allows for
the user interface to run concurrently with the control task (the blinking LED).

### My Solution
Set up a read task. First effort used `scanf()` which works, but only because priority
is just right. If priority of the reading task was higher, it would block but never yield.

Second effort uses pico_sdk function `getchar_timeout_us(0)` which returns immediately
with either a char or "no char ready". If there is a char ready, check if it's a digit.
If it is, accumulate the digit. if not, set the delay if we saw any digits, then reset
the accumulator.

[main.c](https://github.com/kwan3217/rp2040_freertos_tutorial/blob/lesson3/main.c)

