## Lesson 2
* [Web page](https://www.digikey.com/en/maker/projects/introduction-to-rtos-solution-to-part-2-freertos/b3f84c9c9455439ca2dcb8ccfce9dec5) 
* [Video](https://www.youtube.com/watch?v=JIr7Xm_riRs) 

### Challenge

> Using FreeRTOS, create two separate tasks that blink the same LED at two different rates. That means controlling 1 LED with two different delay times.

I interpret this as meaning that the two tasks each have their own led state, toggle it internally and independently,
then set the led to whatever the state is. This might mean that task 2 sets the LED to the same state that task 1 already did. Use two three-digit prime
numbers different by more than a factor of 2 so that it takes a *long* time for the cycle to repeat.

### My Solution
I use the same task function to do both tasks. Task functions take a parameter in the form of a void pointer.
This can be used to pass a pointer to a memory block with an arbitrary structure, but in this problem I want to
pass a delay length, so I just cast the delay length in ticks to a pointer, then cast back to an integer
inside the function.

