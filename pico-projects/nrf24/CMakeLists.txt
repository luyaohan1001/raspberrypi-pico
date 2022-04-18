cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH (/home/luyaohan1001/Projects/raspberrypi-pico/pico-sdk/)
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(nrf24)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# The rest for the project nrf24
# Use following template for added source and libraries:

add_executable(nrf24
 	nrf24.c
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(nrf24 pico_stdlib)

# Enable stdio stream through usb
pico_enable_stdio_usb(nrf24 1)

# Disable stdio stream through uart
pico_enable_stdio_uart(nrf24 0)

# Create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(nrf24)



