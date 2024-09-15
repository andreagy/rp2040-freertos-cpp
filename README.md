# RP2040 FreeRTOS

### ARM Processors and Embedded Operating Systems course exercises
Each exercise is a branch of the original template file provided by the lecturer.

## Exercise 2 Part 1: Activity indicator with binary semaphore
Write a program that creates two tasks: one for reading characters from the serial port and the other for 
indicating received characters on the serial port. Use a binary semaphore to notify serial port activity to the 
indicator task. Note that a single blink sequence (200 ms) takes much longer than transmission time of one 
character (0.1 ms) and only one blink after last character is allowed. 
- Task	1 <br>
Task reads characters from debug serial port using getchar_timeout_us and echoes them back to the serial 
port. When a character is received the task sends an indication (= gives the binary semaphore) to blinker 
task. 
- Task	2	<br>
This task blinks the led once (100 ms on, 100 ms off) when it receives activity indication (= takes the binary 
semaphore). 
