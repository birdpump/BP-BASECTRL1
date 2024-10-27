//#include <iostream>
//#include <sstream>
//#include <cmath>
//#include <cstdio>
//#include <sstream>
//#include <cstdio>
//
//#include <pico/stdlib.h>
//
//#include <RadioLib.h>
//
//#include "hal/RPiPico/PicoHal.h"
//
//#include "utils/telemetry_radio.h"
//
//#define SPI_PORT spi0
//#define SPI_MISO 4
//#define SPI_MOSI 3
//#define SPI_SCK 2
//
//#define RFM_NSS 26
//#define RFM_RST 22
//#define RFM_DIO0 14
//#define RFM_DIO1 15
//
//
//PicoHal* hal = new PicoHal(SPI_PORT, SPI_MISO, SPI_MOSI, SPI_SCK);
//
//SX1262 radio = new Module(hal, RFM_NSS, RFM_DIO0, RFM_RST, RFM_DIO1);
//
//void testRadio(){
//    printf("[SX1276] Initializing ... ");
//    int state = radio.begin();
//    if (state != RADIOLIB_ERR_NONE) {
//        printf("failed, code %d\n", state);
//        return;
//    }
//    printf("success!\n");
//
//    printf("[SX1276] Transmitting packet ... ");
//    state = radio.transmit("Hello World!");
//    if(state == RADIOLIB_ERR_NONE) {
//        printf("success!\n");
//
//        hal->delay(1000);
//    } else {
//        printf("failed, code %d\n", state);
//        return;
//    }
//}