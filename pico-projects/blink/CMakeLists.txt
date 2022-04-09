cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH (/home/luyaohan1001/Projects/raspberrypi-pico/pico-sdk)
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(blink)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# The rest for the project blink
# Use following template for added source and libraries:
# 
add_executable(blink blink.c)
# 
# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(blink pico_stdlib)
# 
# Create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(blink)


