#include <iostream>
#include <sstream>
#include <cmath>
#include <cstdio>

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <RadioLib.h>

#include "tasks/led_task.h"
#include "tasks/radio_tasks.h"
#include "utils/encode_data.h"


#define UART_ID uart0
#define BAUD_RATE 115200
#define TX_PIN PICO_DEFAULT_UART_TX_PIN
#define RX_PIN PICO_DEFAULT_UART_RX_PIN

using namespace std;

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

    vTaskStartScheduler();

    printf("Scheduler failed to start\n");
    while (1);
}
