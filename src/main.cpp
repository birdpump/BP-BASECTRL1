#include <iostream>
#include <sstream>
#include <cmath>
#include <cstdio>

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <RadioLib.h>

#include "f_util.h"
#include "ff.h"
// #include "rtc.h" // TODO figure out what is using this header
#include "hw_config.h"

#include "tasks/led_task.h"
#include "tasks/radio_tasks.h"
#include "utils/encode_data.h"
//#include "utils/telemetry_radio.h"

#define START_BYTE 0xAA
#define TELEMETRY_TYPE 0x01
#define COMMAND_TYPE 0x02
#define UART_ID uart0
#define BAUD_RATE 115200
#define TX_PIN PICO_DEFAULT_UART_TX_PIN
#define RX_PIN PICO_DEFAULT_UART_RX_PIN

using namespace std;

void test_SD() {
    sleep_ms(10000);

    puts("Hello, world!");

    FATFS fs;
    FRESULT fr = f_mount(&fs, "", 1);
    if (FR_OK != fr) {
        panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    }

    // Open a file and write to it
    FIL fil;
    const char *const filename = "filename.txt";
    fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr) {
        panic("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
    }
    if (f_printf(&fil, "Hello, world!\n") < 0) {
        printf("f_printf failed\n");
    }

    // Close the file
    fr = f_close(&fil);
    if (FR_OK != fr) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }

    f_unmount("");
}

void setup() {
    sleep_ms(5000);
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(RX_PIN, GPIO_FUNC_UART);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    initRadio();
}

int main() {
    stdio_init_all();

    setup();

    if (xTaskCreate(initRadioTask, "initRadioTask", 8192, NULL, 4, NULL) != pdPASS) {
        printf("Failed to create initRadio task\n");
        while (1);
    }


//    if (xTaskCreate(baseRadioTX, "baseRadioTX", 8192, NULL, 3, NULL) != pdPASS) {
//        printf("Failed to create Radio task\n");
//        while (1);
//    }
//
//    if (xTaskCreate(baseRadioRX, "baseRadioRX", 8192, NULL, 2, NULL) != pdPASS) {
//        printf("Failed to create Radio task\n");
//        while (1);
//    }

//    if (xTaskCreate(ledTask, "ledTask", 128, NULL, 1, NULL) != pdPASS) {
//        printf("Failed to create Radio task\n");
//        while (1);
//    }

    vTaskStartScheduler();

    printf("Scheduler failed to start\n");
    while (1);
}
