## Lesson 5 - Queues
* [Web page](https://www.digikey.com/en/maker/projects/introduction-to-rtos-solution-to-part-5-freertos-queue-example/72d2b361f7b94e0691d947c7c29a03c9)
* [Video](https://www.youtube.com/watch?v=pHJ3lxOoWeI)

### Challenge
> Use FreeRTOS to create two tasks and two queues.
>
> ![Two tasks using two queues to communicate](challenges/images/challenge5.jpeg)
>
> Task A should print any new messages it receives from Queue 2. Additionally, it
should read any Serial input from the user and echo back this input to the serial
input. If the user enters “delay” followed by a space and a number, it should
send that number to Queue 1.
>
> Task B should read any messages from Queue 1. If it contains a number, it should
update its delay rate to that number (milliseconds). It should also blink an LED
at a rate specified by that delay. Additionally, every time the LED blinks 100
times, it should send the string “Blinked” to Queue 2. You can also optionally send
the number of times the LED blinked (e.g. 100) as part of struct that encapsulates
the string and this number.

### My Solution
This is implemented very directly. Task A could be decomposed into a usb reading task and a
usb printing task, but that's not the spec.

[main.c](https://github.com/kwan3217/rp2040_freertos_tutorial/blob/lesson6/main.c)

