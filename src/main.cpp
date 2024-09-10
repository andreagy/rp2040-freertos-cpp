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

// Event structure
typedef enum {
    EVENT_ROTARY_TURN,
    EVENT_BUTTON_PRESS
} event_type_t;

typedef struct {
    event_type_t type;
    int direction; // 1 for clockwise, -1 for counter-clockwise
} gpio_event_t;

QueueHandle_t eventQueue;

volatile uint32_t last_button_press_time = 0;

void gpio_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    gpio_event_t event;

    // Handle rotary encoder turn (Rot_A rising edge)
    if (gpio == ROT_A_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        event.type = EVENT_ROTARY_TURN;
        event.direction = gpio_get(ROT_B_PIN) ? -1 : 1; // Check Rot_B for direction
        xQueueSendFromISR(eventQueue, &event, &xHigherPriorityTaskWoken);
    }

    // Handle button press (Rot_SW falling edge)
    if (gpio == ROT_SW_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_button_press_time > 250) {
            last_button_press_time = current_time;
            event.type = EVENT_BUTTON_PRESS;
            event.direction = 0;  // No direction for button press
            xQueueSendFromISR(eventQueue, &event, &xHigherPriorityTaskWoken);
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
    gpio_event_t event;

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
        if (xQueueReceive(eventQueue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event.type) {
                case EVENT_BUTTON_PRESS:
                    // Toggle LED state
                    led_on = !led_on;
                    std::cout << "LED " << (led_on ? "ON" : "OFF") << "\n";
                    break;

                case EVENT_ROTARY_TURN:
                    if (led_on) {
                        // Only change frequency if LED is ON
                        blink_frequency += event.direction;
                        if (blink_frequency < 2) blink_frequency = 2;    // Min frequency
                        if (blink_frequency > 200) blink_frequency = 200; // Max frequency
                        std::cout << "Blink frequency: " << blink_frequency << " Hz\n";
                    }
                    break;
            }
        }
    }
}


int main()
{
    stdio_init_all();

    eventQueue = xQueueCreate(10, sizeof(gpio_event_t));
    vQueueAddToRegistry(eventQueue, "GPIO_Event_Queue");

    xTaskCreate(blink_task, "BLINK", 256, (void *) nullptr, 1, NULL);
    xTaskCreate(event_handler_task, "EVENT_HANDLER", 256, (void *) nullptr, 1, NULL);

    vTaskStartScheduler();

    while(1){};
}
