# Add the executable
add_executable(${PROJECT_NAME}
        main.cpp
        tasks/led_task.cpp
        tasks/radio_tasks.cpp
        utils/encode_data.cpp
        utils/telemetry_radio.cpp
        config/hw_config.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib
        include
)

# setup NanoPB
find_package(Nanopb REQUIRED)
nanopb_generate_cpp(TARGET proto config/proto/simple.proto)

# Link libraries
target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        hardware_gpio
        hardware_spi
        hardware_timer
        hardware_pwm
        FreeRTOS-Kernel-Heap4
        RadioLib
        proto
        no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
)

pico_set_program_name(${PROJECT_NAME} ${PROJECT_NAME})
pico_set_program_version(${PROJECT_NAME} "0.1")

# Enable USB output, disable UART output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})