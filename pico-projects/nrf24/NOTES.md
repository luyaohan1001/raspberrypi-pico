In order to enable USB through serial.

1.  In CMakeLists.txt, add the following:

# Enable stdio stream through usb
  pico_enable_stdio_usb(nrf24 1)

# Disable stdio stream through uart
  pico_enable_stdio_uart(nrf24 0)

2. Add tinyusb
 If you do not have https://github.com/raspberrypi/tinyusb inside your lib folder of pico-sdk, then you will get a CMake warning saying it cant find TinyUSB, but it will let you compile without it. All you have to do it download the repo from github, extract it and move the contents into "pico-sdk/lib/tinyusb"

  $ cd <proj-folder>/pico-sdk/lib

  $ git submodule add https://github.com/raspberrypi/tinyusb ./tinyusb

