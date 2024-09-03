#include <string>
#include <iostream>
#include <vector>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pico/stdlib.h"
#include "semphr.h"


#include "hardware/timer.h"
extern "C" {
uint32_t read_runtime_ctr(void) {
    return timer_hw->timerawl;
}
}

SemaphoreHandle_t serial_sem;

struct task_params{
    QueueHandle_t comm_q;
    uint pin;
};

void read_task(void *param) {
    int ch {0};
    while(1) {
        ch = getchar_timeout_us(0);
        if(ch != PICO_ERROR_TIMEOUT && ch != '\n' && ch != '\r') {
            putchar(ch);
            std::cout << ch;

            xSemaphoreGive(serial_sem);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}


void blinker_task(void *param) {
    gpio_init(20);
    gpio_set_dir(20, GPIO_OUT);
    gpio_put(20, 0);
    while(1) {
        if (xSemaphoreTake(serial_sem, portMAX_DELAY) == pdTRUE) {
            gpio_put(20, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(20, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

int main()
{
    stdio_init_all();

    serial_sem = xSemaphoreCreateBinary();

    if (serial_sem != NULL) {
        xSemaphoreTake(serial_sem, 0);
        xTaskCreate(read_task, "SERIAL_READ", 512, (void *) nullptr, tskIDLE_PRIORITY + 1, NULL);
        xTaskCreate(blinker_task, "LED_BLINK", 512, (void *) nullptr, tskIDLE_PRIORITY + 1, NULL);
        vTaskStartScheduler();
    }



    while(1){};
}
