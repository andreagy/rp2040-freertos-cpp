# RP2040 FreeRTOS CPP

## ARM Processors and Embedded Operating Systems course exercises
#### Each exercise is a branch of the original template project provided by the lecturer.

<p><strong>Exercise 1: Queues</strong><br>
Implement a code lock that uses tasks to read and process button presses. Once the correct sequence of buttons are pressed, an LED is switched on.</p>

<p><strong>Exercise 2 Part 1: Semaphores</strong><br>
The program creates two tasks: one for reading characters from the serial port and the other for indicating received characters by LED blinks. A binary semaphore is used to notify serial port activity to the indicator task.</p>

<p><strong>Exercise 2 Part 2: ISR</strong><br>
Rotary switch turns on and off a blinking LED, and the rotary encoder is used to control blinking frequency, but only when the LED is turned on. Two tasks are created: one for receiving and filtering GPIO events from a queue and other for blinking the 
LED.</p>


The original template is based on https://github.com/LearnEmbeddedSystems/rp2040-freertos-template.git
This is a template project for developing FreeRTOS based applications on Raspberry Pi RP2040 based boards. This template uses the "official" RP2040 port from the Raspberry Pi Foundation. 
