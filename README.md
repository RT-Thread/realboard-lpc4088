# RealBoard4088 #

[![Build Status](https://travis-ci.org/RT-Thread/RealBoard4088.png?branch=master)](https://travis-ci.org/RT-Thread/RealBoard4088)

The RealBoard 4088 is a Graphic LCD board with RT-Thread RTOS running inside. The RealBoard 4088 Board is equipped with NXP's Cortex-M4 based LPC4088 microcontroller suitable for a wide range of applications that requires advanced storage,  communication and high quality graphic displays.

## Getting started ##

The examples of realboard 4088 support Keil MDK/GNU GCC/IAR compiler. You can use scons to build all of then and each example are compiled by travis-ci.org continuously.

## Examples List ##

* [basic kernel](software/rtthread_examples/examples/0_base_kernel), it's a basic example for RT-Thread kernel.
* [finsh shell](software/rtthread_examples/examples/1_finsh), the finsh shell example.
* [msh shell](software/rtthread_examples/examples/1_msh), the msh shell example.
* [file system on SDCard](software/rtthread_examples/examples/2_filesystem_sdcard), the file system on the sdcard. 
* [file system on RAM](software/rtthread_examples/examples/2_filesystem_ramfs), the file system on RAM.
* [file system on ROM](software/rtthread_examples/examples/2_filesystem_romfs), the readonly file system on ROM.
* [basic network](software/rtthread_examples/examples/3_networking), the network example.
* [GUI Hello](software/rtthread_examples/examples/4_gui_hello), the GUI Hello World example.
* [GUI button](software/rtthread_examples/examples/4_gui_button), the GUI button example.
* [GUI demo](software/rtthread_examples/examples/4_gui_examples), the GUI example demo.
* [GUI photo](software/rtthread_examples/examples/4_gui_photo_frame), GUI appliation to dispaly photo. 
* [SQLite Demo](software/rtthread_examples/examples/5_sqlite_examples), SQLite on ARM Cortex-M4.
* [JavaScript](software/rtthread_examples/examples/5_js_espruino), JavaScript(ESpruino) on ARM Cortex-M4.
* [USB Device](software/rtthread_examples/examples/8_usb_device_mstorage), the USB Mass Storage example.

## Contributing ##

## License ##

The code of examples of RealBoard 4088 is released under GPLv2 license. 
