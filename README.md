This repository tracks my progress in the [Digikey Introduction to FreeRTOS](https://www.youtube.com/playlist?list=PLEBQazB0HUyQ4hAPU1cJED6t3DU0h34bz).

1. [x] Get a blinky+usb cdc stream running on real RP2040 hardware
2. [ ] Get FreeRTOS installed and get a pure blinky running as a task
3. [ ] Get usb cdc stream working in a task 

# Clone
```
cd ~
git clone https://github.com/kwan3217/rp2040_freertos_tutorial.git
cd rp2040_freertos_tutorial
git submodule update --init --recursive
```

# Toolchain

```
sudo apt update
sudo apt install -y git cmake build-essential gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib python3 libusb-1.0-0-dev
```

# picotool
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

