#include <iostream>
#include <cstring>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "hardware/timer.h"
extern "C" {
uint32_t read_runtime_ctr(void) {
    return timer_hw->timerawl;
}
}

// Pins
const uint SW0_PIN = 9;
const uint LED_PIN = 22;

struct debugEvent {
    const char *format;
    uint32_t data[3];
};

QueueHandle_t log_q;

void debug(const char *format, uint32_t d1, uint32_t d2, uint32_t d3) {
    debugEvent e;
    e.format = format;
    e.data[0] = d1;
    e.data[1] = d2;
    e.data[2] = d3;
    xQueueSend(log_q, &e, portMAX_DELAY);
}

void debugTask(void *param) {
    char buffer[64];
    debugEvent e;

    while (1) {
        if (xQueueReceive(log_q, &e, portMAX_DELAY) == pdTRUE) {
            //get the current timestamp
            TickType_t timestamp = xTaskGetTickCount();

            //format the message with the timestamp
            snprintf(buffer, sizeof(buffer), "[%u] %s", timestamp, e.format);
            printf(buffer, e.data[0], e.data[1], e.data[2]);
        }
    }
}



int main()
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    log_q = xQueueCreate(10, sizeof(debugEvent));
    vQueueAddToRegistry(log_q, "LOG_Q");


    vTaskStartScheduler();

    while(1){};
}
