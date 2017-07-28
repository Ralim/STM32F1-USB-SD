# STM32F103 USB to SD via SPI Example

This is a complete working example (at time of publication) of creating the worlds slowest SD card reader.
This uses the STM32F103X8Tx as found on the "Blue Pill" boards.
This project is setup and tested in the System Workbench for STM32.

## Pinout

Wiring can be setup as follows:

* SD CS = PA4
* SD SCK = PA5
* SD MISO = PA6
* SD MOSI = PA7
* USB D+ = PA12
* USB D- = PA11

USB D+ will need a pullup.
If you are using a breakout board, the usb is probably wired up for you.

You will also need some method of connecting the SD card to the micro, there are plenty of cheap breakout boards available online.

A STM32CubeMX project is also included that the project was originally based on.
This project is C++ and uses the SPI library that I have published previously.

## Useage / Licence

You are free to do whatever you want with this. I do not provide any support or anything, this is mostly provided for learning and reference.

Thanks,
Ben V. Brown
