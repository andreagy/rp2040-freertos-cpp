# RP2040 FreeRTOS

### ARM Processors and Embedded Operating Systems course exercises
Each exercise is a branch of the original template file provided by the lecturer.

## Exercise 2 Part 2 - GPIO interrupts and queue
Implement a program for switching a LED on/off and changing the blinking frequency. The program should work as follows: 
- Rot_Sw, the push button on the rotary encoder shaft is the on/off button. When button is pressed the state of LEDs is toggled. Program must require that button presses that are closer than 250 ms are ignored. 
- Rotary encoder is used to control blinking frequency of the LED. Turning the knob clockwise increases frequency and turning counterclockwise reduces frequency. If the LED is in OFF state turning the knob has no effect. Minimum frequency is 2 Hz and maximum frequency is 200 Hz. 
When frequency is changed it must be printed  
- When LED state is toggled to ON the program must use the frequency at which it was switched off. You must use GPIO interrupts for detecting the encoder turns and button presses and send the button and encoder events to a queue. 
All queues must be registered to queue registry.
Create two tasks: one for receiving and filtering gpio events from the queue and other for blinking the LED.
