#include <iostream>
#include <cstdio>
#include <string>

#include <pico/stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include <semphr.h>

#include <RadioLib.h>
#include "hal/RPiPico/PicoHal.h"

#include "tasks/radio_tasks.h"

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


SemaphoreHandle_t xPacketSemaphore;
SemaphoreHandle_t xRadioMutex;

void setFlag() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xPacketSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void initRadio() {
    printf("[Radio] Initializing Radio...\n");
    int state = radio.begin(902.5, 125.0, 8, 5, 0x36, 22, 14);
    if (state != RADIOLIB_ERR_NONE) {
        printf("[Radio] Initialization Failed, code %d\n", state);
        return;
    }
    printf("[Radio] Initialization Successful\n");

    xRadioMutex = xSemaphoreCreateMutex();

    xPacketSemaphore = xSemaphoreCreateBinary();
    radio.setPacketReceivedAction(setFlag);
}

uint8_t calculateChecksum(const uint8_t *buffer, size_t length) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; i++) {
        checksum ^= buffer[i];
    }
    return checksum;
}


void commandRadio(void *pvParameters) {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    printf("[Radio] Starting listener...\n");
    int state = radio.startReceive();
    if (state != RADIOLIB_ERR_NONE) {
        printf("[Radio] Listening Failed, code %d\n", state);
        return;
    }

    for (;;) {
        if (xSemaphoreTake(xPacketSemaphore, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(xRadioMutex, portMAX_DELAY) == pdTRUE) {
                const size_t len = 80;
                uint8_t data[len];

                // Attempt to read data from the LoRa radio
                int16_t state = radio.readData(data, len);

                if (state == RADIOLIB_ERR_NONE) {
                    printf("[Radio] Received data!\n");

                    // Validate the start byte
                    if (data[0] != START_BYTE) {
                        printf("Invalid start byte\n");
                        continue;
                    }

                    // Extract length, type, and checksum from the received frame
                    uint8_t length = data[1];
                    uint8_t type = data[2];
                    uint8_t payload[length];
                    memcpy(payload, &data[3], length);
                    uint8_t received_checksum = data[3 + length];

                    // Verify checksum
                    if (received_checksum != calculateChecksum(payload, length)) {
                        printf("Checksum validation failed\n");
                        continue;
                    }

                    // Decode payload if the message type matches
                    if (type == COMMAND_TYPE) {
                        uart_putc(UART_ID, START_BYTE);
                        uart_putc(UART_ID, length);
                        uart_putc(UART_ID, TELEMETRY_TYPE);
                        uart_write_blocking(UART_ID, payload, length);
                        uart_putc(UART_ID, calculateChecksum(payload, length));
                    }

                    // Print LoRa metadata
                    printf("[Radio] RSSI:\t\t%.2f dBm\n", radio.getRSSI());
                    printf("[Radio] SNR:\t\t%.2f dB\n", radio.getSNR());
                    printf("[Radio] FreqErr:\t%.2f Hz\n", radio.getFrequencyError());
                } else {
                    printf("Failed to read data, code %d\n", state);
                }

                xSemaphoreGive(xRadioMutex);
            }
        }
    }
}


void commandRadioTask(void *pvParameters) {
    for (;;) {
        if (xSemaphoreTake(xRadioMutex, portMAX_DELAY) == pdTRUE) {
            if (uart_is_readable(UART_ID)) {
                // Check for start byte to begin reading a new frame
                if (uart_getc(UART_ID) == START_BYTE) {
                    uint8_t length = uart_getc(UART_ID); // Read the payload length
                    uint8_t type = uart_getc(UART_ID); // Read the message type

                    uint8_t buffer[64];
                    uart_read_blocking(UART_ID, buffer, length); // Read the payload

                    uint8_t received_checksum = uart_getc(UART_ID); // Read the checksum byte
                    // Verify checksum before sending over LoRa
                    if (received_checksum == calculateChecksum(buffer, length)) {
                        // Construct the frame
                        uint8_t frame[3 + length + 1]; // start byte + length + type + payload + checksum
                        frame[0] = START_BYTE;
                        frame[1] = length;
                        frame[2] = type;
                        memcpy(&frame[3], buffer, length);
                        frame[3 + length] = received_checksum;

                        // Send the frame over LoRa
                        printf("[SX1276] Transmitting packet over LoRa ... ");
                        int state = radio.transmit(frame, sizeof(frame));
                        if (state == RADIOLIB_ERR_NONE) {
                            printf("LoRa transmission successful!\n");
                        } else {
                            printf("LoRa transmission failed, code %d\n", state);
                        }
                    }
                }
            }
            xSemaphoreGive(xRadioMutex);

            vTaskDelay(pdMS_TO_TICKS(10)); // Add a delay to avoid tight looping
        }
    }
}
