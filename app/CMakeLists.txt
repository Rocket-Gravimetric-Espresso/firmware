# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(BOARD nrf52840dk_nrf52840)
set(SHIELD adafruit_2_8_tft_touch_v2)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rocket)

FILE(GLOB_RECURSE app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE src src/lvgl src/lvgl/screens src/lvgl/widgets)
