#include <iostream>
#include <cstring>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "pico/stdlib.h"
#include "timers.h"
#include "PicoOsUart.h"

#include "hardware/timer.h"
extern "C" {
uint32_t read_runtime_ctr(void) {
    return timer_hw->timerawl;
}
}

const uint LED_PIN = 22;

PicoOsUart uart(0, 0, 1, 115200);

int command_count {0};

TimerHandle_t inactivity_timer;
TimerHandle_t led_toggle_timer;

TickType_t last_toggle_time = 0;
int led_interval = 5000; // 5 sec
bool led_on = false;

void process_command(char *command) {
    if (strcmp (command, "help") == 0) {
        uart.send("Available commands:\r\n");
        uart.send("help - display this message\r\n");
        uart.send("interval <seconds> - set LED toggle interval\r\n");
        uart.send("time - show time since last LED toggle\r\n");

    } else if (strncmp (command, "interval ", 9) == 0) {
        int new_interval = atoi(command + 9); // get number after "interval"
        if (new_interval > 0) {
            led_interval = new_interval * 1000;
            xTimerChangePeriod(led_toggle_timer, pdMS_TO_TICKS(led_interval), 0);
            uart.send("New LED interval set\r\n");
        } else {
            uart.send("Invalid interval\r\n");
        }

    } else if (strcmp(command, "time") == 0) {
        TickType_t current_time = xTaskGetTickCount();
        int elapsed_time = (current_time - last_toggle_time) * portTICK_PERIOD_MS / 100;
        uart.send("Time since last toggle: ");
        uart.send(std::to_string(elapsed_time / 10).c_str());
        uart.send(".");
        uart.send(std::to_string(elapsed_time % 10).c_str());
        uart.send(" seconds\r\n");

    } else {
        uart.send("Unknown command: ");
        uart.send(command);
        uart.send("\r\n");
    }
}

void uart_receive_task(void *param) {
    uint8_t buffer[64];
    char command_buffer[64];
    while (true) {
        if (int count = uart.read(buffer, 64, 30); count > 0) {
            uart.write(buffer, count); //show typing

            for (int i = 0; i < count; i++) {
                char current_char = static_cast<char>(buffer[i]);
                command_buffer[command_count] = current_char;

                if (current_char == '\r' || current_char == '\n') {
                    command_buffer[command_count] = '\0';
                    process_command(command_buffer);
                    command_count = 0;

                    xTimerReset(inactivity_timer, 0);

                }else {
                    //increment if valid char
                    command_count++;

                    //ensure buffer doesnt overflow
                    if (command_count >= sizeof(command_buffer) - 1) {
                        command_count = 0;  //reset if overflowed
                    }
                }
            }
        }
    }
}

void inactivity_timer_callback(TimerHandle_t xTimer) {
    command_count = 0;
    uart.send("[Inactive]\r\n");
}

void led_toggle_timer_callback(TimerHandle_t xTimer) {
    last_toggle_time = xTaskGetTickCount(); //update this value
    led_on = !led_on;
    gpio_put(LED_PIN, led_on);
}



int main()
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    inactivity_timer = xTimerCreate(
            "InactivityTimer",
            pdMS_TO_TICKS(30000),
            pdFALSE,
            NULL,
            inactivity_timer_callback);

    led_toggle_timer = xTimerCreate(
            "LedToggleTimer",
            pdMS_TO_TICKS(led_interval),
            pdTRUE,
            NULL,
            led_toggle_timer_callback);

    xTimerStart(led_toggle_timer, 0);
    xTimerStart(inactivity_timer, 0);

    xTaskCreate(uart_receive_task,
                "UART_RECEIVE",
                512,
                (void *) nullptr,
                1,
                NULL);

    vTaskStartScheduler();

    while(1){};
}
