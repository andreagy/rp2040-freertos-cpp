# RP2040 FreeRTOS

## ARM Processors and Embedded Operating Systems course exercises
#### Each exercise is a branch of the original template project provided by the lecturer.

<p><strong>Exercise 1: Queues</strong><br>
Implement a code lock that uses tasks to read and process button presses. Once the correct sequence of buttons are pressed, an LED is switched on.</p>

<p><strong>Exercise 2 Part 1: Semaphores</strong><br>
The program creates two tasks: one for reading characters from the serial port and the other for indicating received characters by LED blinks. A binary semaphore is used to notify serial port activity to the indicator task.</p>

<p><strong>Exercise 2 Part 2: ISR</strong><br>
Rotary switch turns on and off a blinking LED, and the rotary encoder is used to control blinking frequency, but only when the LED is turned on. Two tasks are created: one for receiving and filtering GPIO events from a queue and other for blinking the 
LED.</p>

<p><strong>Exercise 3: Timers</strong><br>
Read commands from the serial port using the interrupt driven FreeRTOS UART driver provided. User can enter 3 commands: "help" to display commands, "interval <seconds>" to set LED toggle interval, and "time" to display time since the last LED toggle. The program creates two timers: one for detecting user input inactivity, and one for toggling an LED. </p>

<p><strong>Exercise 4: Event Groups, Part 1</strong><br>
Implement a program with 4 tasks and an event group. Task 1 waits for a button press, sets an event bit, and then prints elapsed ticks with a random delay. Tasks 2 and 3 wait for the event bit and then they loop, printing elapsed ticks with random delays. Task 4 handles debug printing. Tasks 1-3 have higher priority than Task 4." </p>

<p><strong>Exercise 4: Event Groups, Part 2</strong><br>
Implement a program with 5 tasks and an event group. Tasks 1–3 monitor buttons, setting event bits and printing debug messages on press/release. Holding a button blocks the task loop. Task 4 is a watchdog that prints 'OK' with elapsed ticks when all tasks run within 30 seconds. If any task fails, it prints 'Fail' with the task number and suspends. Task 5 handles debug printing at a lower priority. </p>



The original template is based on https://github.com/LearnEmbeddedSystems/rp2040-freertos-template.git
This is a template project for developing FreeRTOS based applications on Raspberry Pi RP2040 based boards. This template uses the "official" RP2040 port from the Raspberry Pi Foundation. 
