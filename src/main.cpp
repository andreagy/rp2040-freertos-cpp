#include <iostream>
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
const uint ROT_A_PIN = 10;
const uint ROT_B_PIN = 11;
const uint ROT_SW_PIN = 12;
const uint LED_PIN = 22;

SemaphoreHandle_t gpio_sem;
int blink_frequency = 2; // Start frequency at 2 Hz
bool led_on = false;     // LED initially OFF

volatile uint32_t last_button_press_time = 0;

void gpio_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Handle rotary encoder turn (Rot_A rising edge)
    if (gpio == ROT_A_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        int direction = gpio_get(ROT_B_PIN) ? -1 : 1;  // Check Rot_B for direction
        blink_frequency += direction;
        if (blink_frequency < 2) blink_frequency = 2;    // Min frequency
        if (blink_frequency > 200) blink_frequency = 200; // Max frequency
        std::cout << "Blink frequency: " << blink_frequency << " Hz\n";
    }

    // Handle button press (Rot_SW falling edge)
    if (gpio == ROT_SW_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_button_press_time > 250) {
            last_button_press_time = current_time;
            xSemaphoreGiveFromISR(gpio_sem, &xHigherPriorityTaskWoken);
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void blink_task(void *param)
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true) {
        if (led_on) {
            // LED blink logic based on current frequency
            gpio_put(LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(500 / blink_frequency));
            gpio_put(LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(500 / blink_frequency));
        } else {
            // Keep LED off
            gpio_put(LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100)); // Idle
        }
    }
}

void event_handler_task(void *param) {
    const uint delay = pdMS_TO_TICKS(250);  // 250 ms debounce delay

    // Initialize button and rotary encoder pins
    gpio_init(ROT_SW_PIN);
    gpio_set_dir(ROT_SW_PIN, GPIO_IN);
    gpio_pull_up(ROT_SW_PIN);
    gpio_set_irq_enabled_with_callback(ROT_SW_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_init(ROT_A_PIN);
    gpio_set_dir(ROT_A_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(ROT_A_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    gpio_init(ROT_B_PIN);
    gpio_set_dir(ROT_B_PIN, GPIO_IN);

    while (true) {
        if (xSemaphoreTake(gpio_sem, portMAX_DELAY) == pdTRUE) {
            // Button pressed, toggle LED state
            led_on = !led_on;
            std::cout << "LED " << (led_on ? "ON" : "OFF") << "\n";
        }
    }
}


int main()
{
    stdio_init_all();

    gpio_sem = xSemaphoreCreateBinary();
    xTaskCreate(blink_task, "BLINK", 256, (void *) nullptr, 1, NULL);
    xTaskCreate(event_handler_task, "EVENT_HANDLER", 256, (void *) nullptr, 1, NULL);

    vTaskStartScheduler();

    while(1){};
}
