#include <iostream>
#include <cstdio>
#include <string>

#include "FreeRTOS.h"
#include "task.h"

#include "tasks/radio_task.h"

#include "pico/stdlib.h"

#include <RadioLib.h>

#include "hal/RPiPico/PicoHal.h"

using namespace std;

#define SPI_PORT spi1
#define SPI_MISO 12
#define SPI_MOSI 11
#define SPI_SCK 10

#define RFM_NSS 3
#define RFM_RST 15
#define RFM_DIO1 20
#define RFM_DIO2 2


PicoHal* hal = new PicoHal(SPI_PORT, SPI_MISO, SPI_MOSI, SPI_SCK);
SX1262 radio = new Module(hal, RFM_NSS, RFM_DIO1, RFM_RST, RFM_DIO2);

void radioTask(void *pvParameters) {
    printf("[SX1262] Initializing ... \n");
    int state = radio.begin();

    printf("[SX1262] Initializing 2 ... \n");
    if (state != RADIOLIB_ERR_NONE) {
        printf("failed, code %d\n", state);
        return;
    }
    printf("success!\n");

    printf("[SX1262] Starting to listen ... ");
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
        printf("success!");
    } else {
        printf("failed, code %d\n", state);
        return;
    }

    for (;;) {
        const size_t len = 80;
        uint8_t data[len];

        int16_t state = radio.readData(data, 0);

        if (state == RADIOLIB_ERR_NONE) {
            printf("[SX1262] Received packet!\n");

            std::cout << "Data as string: " << reinterpret_cast<char *>(data) << std::endl;

            printf("[SX1262] RSSI:\t\t%.2f dBm\n", radio.getRSSI());

            printf("[SX1262] SNR:\t\t%.2f dB\n", radio.getSNR());

        } else {
            printf("failed, code %d\n", state);
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(800));
    }

}