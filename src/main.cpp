#include <string>
#include <iostream>
#include <vector>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pico/stdlib.h"

using namespace std;


#include "hardware/timer.h"
extern "C" {
uint32_t read_runtime_ctr(void) {
    return timer_hw->timerawl;
}
}

struct task_params{
    QueueHandle_t comm_q;
    uint pin;
};

void button_read_task(void *param) {
    task_params *tpr = (task_params *) param;
    const uint button_pin = tpr->pin;

    gpio_init(button_pin);
    gpio_set_dir(button_pin, GPIO_IN);
    gpio_pull_up(button_pin);

    while (true) {
        if (!gpio_get(button_pin)) {  // Button pressed
            while (!gpio_get(button_pin)) {
                vTaskDelay(pdMS_TO_TICKS(50)); // Debounce delay
            }
            uint32_t button_value = button_pin;
            xQueueSendToBack(tpr->comm_q, static_cast<void *>(&button_value), portMAX_DELAY);

        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Polling delay
    }
}

void process_button_task(void *param) {
    task_params *tpr = (task_params *) param;

    uint32_t button_value;


    gpio_init(20);
    gpio_set_dir(20, GPIO_OUT);
    gpio_put(20, 0);  // LED OFF

    gpio_init(21);
    gpio_set_dir(21, GPIO_OUT);
    gpio_put(21, 0);  // LED OFF

    gpio_init(22);
    gpio_set_dir(22, GPIO_OUT);
    gpio_put(22, 0);  // LED OFF

    while(1) {
        if (xQueueReceive(tpr->comm_q, static_cast<void *>(&button_value), pdMS_TO_TICKS(500)) == pdTRUE) {
            if (button_value == 9) {
                gpio_put(20, 1);
                vTaskDelay(1000);
                gpio_put(20, 0);
            } else if (button_value == 8) {
                gpio_put(21, 1);
                vTaskDelay(1000);
                gpio_put(21, 0);
            } else if (button_value == 7) {
                gpio_put(22, 1);
                vTaskDelay(1000);
                gpio_put(22, 0);
            }
        }

    }
}

int main()
{
    stdio_init_all();

    QueueHandle_t button_queue = xQueueCreate(5, sizeof(uint32_t));

    static task_params sw0_params = { .comm_q = button_queue, .pin = 9 };
    static task_params sw1_params = { .comm_q = button_queue, .pin = 8 };
    static task_params sw2_params = { .comm_q = button_queue, .pin = 7 };

    static task_params process_param = {.comm_q = button_queue, .pin = 8};

    vQueueAddToRegistry(button_queue, "BUTTON_Q");
    xTaskCreate(button_read_task, "BUTTON_SW0", 512, (void *) &sw0_params, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(button_read_task, "BUTTON_SW1", 512, (void *) &sw1_params, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(button_read_task, "BUTTON_SW2", 512, (void *) &sw2_params, tskIDLE_PRIORITY + 1, NULL);

    // Create button processing task
    xTaskCreate(process_button_task, "PROCESS", 1024, (void *) &process_param, tskIDLE_PRIORITY + 2, NULL);

    vTaskStartScheduler();

    while(1){};
}
