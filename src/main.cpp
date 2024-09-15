#include <iostream>
#include <cstring>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "hardware/timer.h"
#include "event_groups.h"

extern "C" {
uint32_t read_runtime_ctr(void) {
    return timer_hw->timerawl;
}
}

// Pins
const uint SW0_PIN = 9;
const uint LED_PIN = 22;

// Event bits
#define TASK1_BIT   (1UL << 0UL) // zero shift for bit0
#define TASK2_BIT   (1UL << 1UL) // 1 shift for flag  bit 1
#define TASK3_BIT   (1UL << 2UL) // 2 shift for flag bit 2

struct debugEvent {
    const char *format;
    uint32_t data[3];
};

QueueHandle_t log_q;
EventGroupHandle_t event_group;

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

void button_task(void *param) {
    gpio_init(SW0_PIN);
    gpio_set_dir(SW0_PIN, GPIO_IN);
    gpio_pull_up(SW0_PIN);

    while (true) {
        if (!gpio_get(SW0_PIN)) {
            while (!gpio_get(SW0_PIN)) {
                uint32_t button_value = SW0_PIN;
                //xQueueSendToBack(log_q, static_cast<void *>(&button_value), portMAX_DELAY);
                xEventGroupSetBits(event_group, TASK1_BIT);
            }
            vTaskDelay(pdMS_TO_TICKS(50)); // Debounce delay
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Polling delay
    }
}



int main()
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    log_q = xQueueCreate(10, sizeof(debugEvent));
    vQueueAddToRegistry(log_q, "LOG_Q");

    event_group = xEventGroupCreate();


    vTaskStartScheduler();

    while(1){};
}
