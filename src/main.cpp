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

#define TASK_RUN_TIMEOUT pdMS_TO_TICKS(30000) // 30 seconds

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
TaskHandle_t watchdogTaskHandle;

// Global to track the last runtime of tasks
TickType_t last_run_times[3] = {0}; // buttonTask1-2-3 -> 0, 1, 2

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
        {SW1_PIN, &SW1_count, &last_SW1_press_time, TASK2_BIT},
        {SW2_PIN, &SW2_count, &last_SW2_press_time, TASK3_BIT}
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

void buttonTask1(void *param) {

    create_button(buttons[0].pin);
    bool previous_state = true; // assuming the button is released (high)

    while (true) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        bool current_state = gpio_get(buttons[0].pin); //true if button is not pressed

        if (previous_state == false && current_state == true) {
            // button was pressed and now released
            if (current_time - *buttons[0].last_button_press_time >= 250) {
                *buttons[0].last_button_press_time = current_time;
                (*buttons[0].counter)++;
                debug("Button: %d pressed. Count: %d\n", 0, *buttons[0].counter, 0);
                xEventGroupSetBits(event_group, buttons[0].bit);
                last_run_times[0] = xTaskGetTickCount();

                // Resume watchdog when event bit is set
                vTaskResume(watchdogTaskHandle);
            }
        }
        previous_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(10)); // Polling delay
    }
}

void buttonTask2(void *param) {

    create_button(buttons[1].pin);
    bool previous_state = true; // assuming the button is released (high)

    while (true) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        bool current_state = gpio_get(buttons[1].pin); //true if button is not pressed

        if (previous_state == false && current_state == true) {
            if (current_time - *buttons[1].last_button_press_time >= 250) {
                *buttons[1].last_button_press_time = current_time;
                (*buttons[1].counter)++;
                debug("Button: %d pressed. Count: %d\n", 1, *buttons[1].counter, 0);
                xEventGroupSetBits(event_group, buttons[1].bit);
                last_run_times[1] = xTaskGetTickCount();

                // Resume watchdog when event bit is set
                vTaskResume(watchdogTaskHandle);
            }
        }
        previous_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(10)); // Polling delay
    }
}

void buttonTask3(void *param) {

    create_button(buttons[2].pin);
    bool previous_state = true; // assuming the button is released (high)

    while (true) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        bool current_state = gpio_get(buttons[2].pin); //true if button is not pressed

        if (previous_state == false && current_state == true) {
            if (current_time - *buttons[2].last_button_press_time >= 250) {
                *buttons[2].last_button_press_time = current_time;
                (*buttons[2].counter)++;
                debug("Button: %d pressed. Count: %d\n", 2, *buttons[2].counter, 0);
                xEventGroupSetBits(event_group, buttons[2].bit);
                last_run_times[2] = xTaskGetTickCount();

                // Resume watchdog when event bit is set
                vTaskResume(watchdogTaskHandle);
            }
        }
        previous_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(10)); // Polling delay
    }
}

void watchdogTask (void *param) {
    TickType_t last_print_time = xTaskGetTickCount();
    while (true) {
        TickType_t current_time = xTaskGetTickCount();
        bool all_tasks_ok = true;

        // Check if each task has run in the last 30 seconds
        for (int i = 0; i < 3; i++) {
            if ((current_time - last_run_times[i]) > TASK_RUN_TIMEOUT) {
                all_tasks_ok = false;
                debug("Fail: Task %d missed deadline\n", i+1, 0, 0);
            }
        }

        if (all_tasks_ok) {
            TickType_t elapsed_time = current_time - last_print_time;
            debug("OK. Elapsed ticks: %u\n", (unsigned) elapsed_time, 0, 0);
            last_print_time = current_time;
        } else {
            debug("Watchdog suspending itself\n", 0, 0, 0);
            vTaskSuspend(NULL);
        }

        xEventGroupWaitBits(
                event_group,
                TASK1_BIT | TASK2_BIT | TASK3_BIT,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY);


        vTaskDelay(pdMS_TO_TICKS(30000)); // Check every 30 seconds
    }
}


int main() {
    stdio_init_all();

    syslog_q = xQueueCreate(10, sizeof(debugEvent));
    vQueueAddToRegistry(syslog_q, "SYSLOG_Q");

    event_group = xEventGroupCreate();

    xTaskCreate(buttonTask1, "TASK1", 512, (void *) nullptr, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(buttonTask2, "TASK2", 512, (void *) nullptr, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(buttonTask3, "TASK3", 512, (void *) nullptr, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(watchdogTask, "TASK4", 512, (void *) nullptr, tskIDLE_PRIORITY + 3, &watchdogTaskHandle);
    xTaskCreate(debugTask, "TASK5", 512, (void *) nullptr, tskIDLE_PRIORITY + 1, NULL);


    vTaskStartScheduler();

    while (1) {};
}
