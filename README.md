This repository tracks my progress in the [Digikey Introduction to FreeRTOS](https://www.youtube.com/playlist?list=PLEBQazB0HUyQ4hAPU1cJED6t3DU0h34bz).

1. [x] Get a blinky+usb cdc stream running on real RP2040 hardware
2. [x] Get FreeRTOS installed and get a pure blinky running as a task
3. [x] Get usb cdc stream working in a task 

# Setup

## Clone
```
cd ~
git clone https://github.com/kwan3217/rp2040_freertos_tutorial.git
cd rp2040_freertos_tutorial
git submodule update --init --recursive
```

## Toolchain

```
sudo apt update
sudo apt install -y git cmake build-essential gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib python3 libusb-1.0-0-dev
```

## picotool
I don't think I've used picotool yet.
```
cd ~
git clone https://github.com/raspberrypi/picotool.git
cd picotool
mkdir build
cd build
PICO_SDK_PATH=~/rp2040_freertos_tutorial/pico-sdk/ cmake ..
make
sudo make install
```

## Build and Upload
To build an image:

```
cd rp2040_freertos_tutorial
rm -rfv build
mkdir build
cd build
cmake .. -DCMAKE_VERBOSE_MAKEFILE=OFF # or ON to have make print the commands before each is executed
make -j9 # make with 9 threads, generally use 1 more than cores
```
This builds build/rp2040_freertos_tutorial.uf2

To upload the image:
* Unplug the pico USB
* Hold down the pico BOOTSEL
* Plug in USB. Pico will enumerate as a mass storage device, `/media/<user>/RPI-RP2` in Linux
* Copy the uf2 to the Pico `cp -av build/rp2040_freertos_tutorial.uf2 /media/$USER/RPI-RP2`
When the copy finishes, the Pico will restart running the new firmware.

# Lessons

## Lesson 0 - fight with compilers and toolchains
The process is:
* get pico-sdk
* write a blinky in `main.c`
* Fight with the cmakelists.txt until it builds and works (fighting complete). Tagged as `lesson0_blinky_no_freertos`
* get FreeRTOS
* Fight with the cmakelists.txt until it builds and works with freertos even though it isn't used yet (fighting complete). Tagged as `lesson0_blinky_freertos_unused`
  * The biggest thing was to include the correct FreeRTOS *.cmake file
* Update main.c to actually start some tasks -- one to blink the LED and one to print to USB stream
* Fight with the cmakelists.txt until it builds and works with actual freertos tasks (fighting complete). Tagged as `lesson0_blinky_freertos`



## Lesson 1 - Why an RTOS?
No code for this lesson

## Lesson 2 - Two tasks
### Challenge

> Using FreeRTOS, create two separate tasks that blink the same LED at two different rates. That means controlling 1 LED with two different delay times.

I interpret this as meaning that the two tasks each have their own led state, toggle it internally and independently, 
then set the led to whatever the state is. This might mean that task 2 sets the LED to the same state that task 1 already did. Use two three-digit prime
numbers different by more than a factor of 2 so that it takes a *long* time for the cycle to repeat.



