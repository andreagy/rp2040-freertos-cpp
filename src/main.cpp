#include <iostream>
#include <cstring>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <random>
#include "hardware/timer.h"
#include "event_groups.h"

extern "C" {
uint32_t read_runtime_ctr(void) {
    return timer_hw->timerawl;
}
}

//RNG setup
std::random_device rd;  // obtain a random number from hardware
std::mt19937 gen(rd()); // seed the generator
std::uniform_int_distribution<> distr(1000, 2000); //define range in ms

// Pins
const uint SW0_PIN = 9;
const uint SW1_PIN = 8;
const uint SW2_PIN = 7;

// Event bits
#define TASK1_BIT   (1UL << 0UL) // zero shift for bit0
#define TASK2_BIT   (1UL << 1UL) // 1 shift for flag  bit 1
#define TASK3_BIT   (1UL << 2UL) // 2 shift for flag bit 2

struct debugEvent {
    const char *format;
    uint32_t data[3];
};

QueueHandle_t syslog_q;
EventGroupHandle_t event_group;

struct Button {
    uint pin;
    int *counter;
    uint32_t *last_button_press_time;
    EventBits_t bit;
};

int SW0_count{0};
int SW1_count{0};
int SW2_count{0};

uint32_t last_SW0_press_time{0};
uint32_t last_SW1_press_time{0};
uint32_t last_SW2_press_time{0};


Button buttons[] = {
        {SW0_PIN, &SW0_count, &last_SW0_press_time, TASK1_BIT},
        {SW1_PIN, &SW1_count, &last_SW1_press_time, TASK1_BIT},
        {SW2_PIN, &SW2_count, &last_SW2_press_time, TASK1_BIT}
};


void create_button(int gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

void debug(const char *format, uint32_t d1, uint32_t d2, uint32_t d3) {
    debugEvent e{};
    e.format = format;
    e.data[0] = d1;
    e.data[1] = d2;
    e.data[2] = d3;
    xQueueSend(syslog_q, &e, portMAX_DELAY);
}

void debugTask(void *param) {
    char buffer[64];
    debugEvent e{};

    while (1) {
        if (xQueueReceive(syslog_q, &e, portMAX_DELAY) == pdTRUE) {
            //get the current timestamp
            TickType_t timestamp = xTaskGetTickCount();

            //format the message with the timestamp
            snprintf(buffer, sizeof(buffer), "[%u] %s", timestamp, e.format);
            printf(buffer, e.data[0], e.data[1], e.data[2]);
        }
    }
}

void buttonTask(void *param) {

    for (auto &button: buttons) {
        create_button(button.pin);
    }

    while (true) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        for (int i = 0; i < 3; i++) {
            if (!gpio_get(buttons[i].pin)) {
                if (current_time - *buttons[i].last_button_press_time >= 250) {
                    *buttons[i].last_button_press_time = current_time;
                    (*buttons[i].counter)++;
                    debug("Button: %d pressed. Count: %d\n", i, *buttons[i].counter, 0);
                    xEventGroupSetBits(event_group, buttons[i].bit);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Polling delay
    }
}

void task2(void *param) {
    const int task_number = 2;
    TickType_t last_print_time = xTaskGetTickCount();

    while (true) {
        // Wait for TASK1_BIT to be set
        xEventGroupWaitBits(event_group, TASK1_BIT, pdTRUE, pdTRUE, portMAX_DELAY);

        while (true) {
            TickType_t current_time = xTaskGetTickCount();
            TickType_t elapsed_ticks = current_time - last_print_time;

            debug("Task %d: Elapsed ticks: %u\n", task_number, (unsigned) elapsed_ticks, 0);
            last_print_time = current_time;

            // Generate random delay
            TickType_t random_delay = pdMS_TO_TICKS(distr(gen));
            vTaskDelay(random_delay);

            break;
        }
    }
}


void task3(void *param) {
    const int task_number = 3;
    TickType_t last_print_time = xTaskGetTickCount();

    while (true) {
        // Wait for TASK1_BIT to be set
        xEventGroupWaitBits(event_group, TASK1_BIT, pdTRUE, pdTRUE, portMAX_DELAY);

        while (true) {
            TickType_t current_time = xTaskGetTickCount();
            TickType_t elapsed_ticks = current_time - last_print_time;


            debug("Task %d: Elapsed ticks: %u\n", task_number, (unsigned) elapsed_ticks, 0);
            last_print_time = current_time;

            // Generate random delay
            TickType_t random_delay = pdMS_TO_TICKS(distr(gen));
            vTaskDelay(random_delay);

            break;
        }
    }
}


int main() {
    stdio_init_all();

    syslog_q = xQueueCreate(10, sizeof(debugEvent));
    vQueueAddToRegistry(syslog_q, "SYSLOG_Q");

    event_group = xEventGroupCreate();

    xTaskCreate(buttonTask, "TASK1", 512, (void *) nullptr, tskIDLE_PRIORITY + 3, NULL);
    xTaskCreate(task2, "TASK2", 512, (void *) nullptr, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(task3, "TASK3", 512, (void *) nullptr, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(debugTask, "TASK4", 512, (void *) nullptr, tskIDLE_PRIORITY + 1, NULL);


    vTaskStartScheduler();

    while (1) {};
}
