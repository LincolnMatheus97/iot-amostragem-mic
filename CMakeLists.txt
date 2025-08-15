# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(iot-amostragem-mic C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(iot-amostragem-mic
    src/iot_amostragem_mic.cpp
    src/connection/cliente_ws.cpp
    src/utils/wifi/wifi.c
    src/utils/buzzer/buzzer.c
    src/utils/displayOLED/display.c
    src/utils/dma/dma.c
    src/utils/matriz/matriz.c
    src/utils/microfone/mic.c
    lib/ssd1306/ssd1306.c
    lib/websocket/ws.cpp
)

add_subdirectory(
    lib/RP2040-WS2812B-Animation
)

pico_set_program_name(iot-amostragem-mic "iot-amostragem-mic")
pico_set_program_version(iot-amostragem-mic "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(iot-amostragem-mic 0)
pico_enable_stdio_usb(iot-amostragem-mic 1)

# Add the standard library to the build
target_link_libraries(iot-amostragem-mic
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_multicore
    hardware_dma
    hardware_adc
    hardware_i2c
    hardware_pwm
    ws2812b_animation
)

pico_generate_pio_header(iot-amostragem-mic ${CMAKE_CURRENT_LIST_DIR}/lib/RP2040-WS2812B-Animation/ws2812.pio)

target_sources(iot-amostragem-mic PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/lib/RP2040-WS2812B-Animation/ws2812b_animation.c
)

# Add the standard include files to the build
target_include_directories(iot-amostragem-mic PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/src/utils
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/displayOLED
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/dma
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/microfone
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/matriz
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/buzzer
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/wifi
        ${CMAKE_CURRENT_LIST_DIR}/src/connection
        ${CMAKE_CURRENT_LIST_DIR}/lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/ssd1306
        ${CMAKE_CURRENT_LIST_DIR}/lib/websocket
        ${CMAKE_CURRENT_LIST_DIR}/lib/RP2040-WS2812B-Animation/inc
        ${CMAKE_CURRENT_LIST_DIR}/lib/RP2040-WS2812B-Animation/inc/CP0-EU
        ${CMAKE_CURRENT_LIST_DIR}/lib/RP2040-WS2812B-Animation/inc/utf8-iterator/source
        
)

# Add any user requested libraries
target_link_libraries(iot-amostragem-mic

        )

pico_add_extra_outputs(iot-amostragem-mic)


# Configuração da biblioteca ws2812b_animation
set(TARGET_NAME "ws2812b_animation")

if (NOT TARGET ${TARGET_NAME})
    add_library(${TARGET_NAME} INTERFACE)

    target_sources(${TARGET_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/lib/RP2040-WS2812B-Animation/ws2812b_animation.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/RP2040-WS2812B-Animation/inc/utf8-iterator/source/utf-8.c
    )

    target_include_directories(${TARGET_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/lib/RP2040-WS2812B-Animation/inc
        ${CMAKE_CURRENT_LIST_DIR}/lib/RP2040-WS2812B-Animation/inc/CP0-EU
        ${CMAKE_CURRENT_LIST_DIR}/lib/RP2040-WS2812B-Animation/inc/utf8-iterator/source
    )

    target_link_libraries(${TARGET_NAME} INTERFACE
        hardware_pio
    )
endif()