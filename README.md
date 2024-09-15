# RP2040 FreeRTOS CPP

### ARM Processors and Embedded Operating Systems course exercises
Each exercise is a branch of the original template file provided by the lecturer.

## Exercise 3: Timers
Implement a program that reads commands from the serial port using the provided interrupt driven 
FreeRTOS uart driver. The program creates two timers: one for inactivity monitoring and one for toggling 
the green led. If no characters are received in 30 seconds all the characters received so far are discarded 
and the program prints “[Inactive]”. When a character is received the inactivity timer is started/reset. 
When enter is pressed the received character are processed in a command interpreter. The commands are: 
- help – display usage instructions 
- interval <number> - set the led toggle interval (default is 5 seconds) 
- time – prints the number of seconds with 0.1s accuracy since the last led toggle

If no valid command is found the program prints: “unknown command”.
