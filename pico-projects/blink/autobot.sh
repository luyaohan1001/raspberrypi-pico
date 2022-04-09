# File      :     autobot.sh
# Author    :     Luyao Han
# Email     :     luyaohan1001@gmail.com
# Brief     :     A script to initialize pico project environment by importing cmake configuration files.
# Date      :     04-09-2022
# Copyright (C) 2022-2122 Luyao Han. The following code may be shared or modified for personal use / non-commercial use.
# -------- -------- -------- -------- -------- -------- -------- -------- -------- -------- -------- --------

# copy cmake configuration from pico-sdk directory to current dicretory.
if [[ ! -f pico_sdk_import.cmake ]]; then
	cp ../../pico-sdk/external/pico_sdk_import.cmake .
fi

# Set environment variable for cmake build.
export PICO_SDK_PATH=/home/luyaohan1001/Projects/raspberrypi-pico/pico-sdk/

# Get project name 
dir=`pwd`
project=`basename $dir`

# Create CMakeList.txt if not exist.
if [[ ! -f CMakeLists.txt ]]; then
	touch CMakeLists.txt

	init_text+="cmake_minimum_required(VERSION 3.13)\n"
	init_text+="\n"
	init_text+="# initialize the SDK based on PICO_SDK_PATH ($PICO_SDK_PATH)\n"
	init_text+="# note: this must happen before project()\n"
	init_text+="include(pico_sdk_import.cmake)\n"
	init_text+="\n"
	init_text+="project($project)\n"
	init_text+="\n"
	init_text+="# initialize the Raspberry Pi Pico SDK\n"
	init_text+="pico_sdk_init()\n"
	init_text+="\n"
	init_text+="# The rest for the project $project\n"
	init_text+="# Use following template for added source and libraries:\n"
	init_text+="# \n"
	init_text+="# add_executable(hello-world\n"
	init_text+="#	\thello-world.c\n"
	init_text+="#)\n"
	init_text+="# \n"
	init_text+="# Add pico_stdlib library which aggregates commonly used features\n"
	init_text+="# target_link_libraries(hello-world pico_stdlib)\n"
	init_text+="# \n"
	init_text+="# Create map/bin/hex/uf2 file in addition to ELF.\n"
	init_text+="# pico_add_extra_outputs(hello-world)\n"
	init_text+="\n"



	# -e interprets "\n"
	echo -e $init_text >> CMakeLists.txt
fi

# Create Cmake build directory if not already exist.
if [[ ! -d build ]]; then
	mkdir build

fi

# Compile
cd build
cmake .. -DPICO_SDK_PATH=$PICO_SDK_PATH
