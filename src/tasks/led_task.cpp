#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

#include "tasks/led_task.h"

using namespace std;

void ledTask(void *pvParameters) {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    for (;;) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(100));

        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(400));
    }
}