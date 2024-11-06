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

#define START_BYTE 0xAA
#define TELEMETRY_TYPE 0x01
#define COMMAND_TYPE 0x02
#define UART_ID uart0
#define BAUD_RATE 115200
#define TX_PIN PICO_DEFAULT_UART_TX_PIN
#define RX_PIN PICO_DEFAULT_UART_RX_PIN


PicoHal *hal = new PicoHal(SPI_PORT, SPI_MISO, SPI_MOSI, SPI_SCK);
SX1262 radio = new Module(hal, RFM_NSS, RFM_DIO1, RFM_RST, RFM_DIO2);


uint8_t calculateChecksum(const uint8_t *buffer, size_t length) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; i++) {
        checksum ^= buffer[i];
    }
    return checksum;
}


void radioTask(void *pvParameters) {
    printf("[SX1262] Initializing ... \n");
    int state = radio.begin(902.5, 125.0, 8, 5, 0x36, 22, 14);
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
            std::cout << "Data: " << reinterpret_cast<char *>(data) << std::endl;

            printf("[SX1262] RSSI:\t\t%.2f dBm\n", radio.getRSSI());

            printf("[SX1262] SNR:\t\t%.2f dB\n", radio.getSNR());

            printf("[SX1262] FreqErr:\t\t%.2f Hz\n", radio.getFrequencyError());


        } else {
            printf("failed, code %d\n", state);
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void commandRadioTask(void *pvParameters) {
    printf("[SX1262] Initializing ... ");
    int state = radio.begin(902.5, 125.0, 8, 5, 0x36, 22, 14);
    if (state != RADIOLIB_ERR_NONE) {
        printf("failed, code %d\n", state);
        return;
    }
    printf("success!\n");

    for (;;) {
        if (uart_is_readable(UART_ID)) {
            // Check for start byte to begin reading a new frame
            if (uart_getc(UART_ID) == START_BYTE) {
                uint8_t length = uart_getc(UART_ID);  // Read the payload length
                uint8_t type = uart_getc(UART_ID);    // Read the message type

                uint8_t buffer[64];
                uart_read_blocking(UART_ID, buffer, length);  // Read the payload

                uint8_t received_checksum = uart_getc(UART_ID);  // Read the checksum byte
                // Verify checksum before sending over LoRa
                if (received_checksum == calculateChecksum(buffer, length)) {
                    // Construct the frame
                    uint8_t frame[3 + length + 1];  // start byte + length + type + payload + checksum
                    frame[0] = START_BYTE;
                    frame[1] = length;
                    frame[2] = type;
                    memcpy(&frame[3], buffer, length);
                    frame[3 + length] = received_checksum;

                    // Send the frame over LoRa
                    printf("[SX1276] Transmitting packet over LoRa ... ");
                    state = radio.transmit(frame, sizeof(frame));
                    if (state == RADIOLIB_ERR_NONE) {
                        printf("LoRa transmission successful!\n");
                    } else {
                        printf("LoRa transmission failed, code %d\n", state);
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));  // Add a delay to avoid tight looping
    }
}
