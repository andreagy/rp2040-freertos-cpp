# RP2040 FreeRTOS

### ARM Processors and Embedded Operating Systems course exercises
Each exercise is a branch of the original template file provided by the lecturer.

## Exercise 4: Event Groups
#### Debug	printing	
Implement a debug print task for printing debug/log messages. The debug print task must be the only task that uses 
standard output. Priority of debug task must be idle task priority + 1. Other tasks must run on a higher priority 
than debug print task. Tasks communicate the message to print by sending data to a queue. 
The debug print task waits on a queue. Queue items contain a constant string and three numbers. The string 
must be a literal or other type of string that does not change at run time. The string is a format string that is 
accepted by printf/sprinf. When debug() is called (see examples below) it constructs a debug event and 
sends it to queue. For example we could call:  

```debug("Button: %d pressed. Count: %d\n", 2, count, 0);```

Function must always send a timestamp to the queue with the message to print. The print task prints the 
timestamp at the beginning of the line followed by the message 
The queue maintains ordering of the items so the debug messages will print out in the same order as they 
were sent to the queue.

### Part 1: 
Implement a program with four tasks and an event group. Task 1 waits for user to press a button. When the 
button is pressed task 1 sets bit 0 of the event group. Tasks 2 and 3 wait on the event bit with an infinite 
timeout. When the bit is set the tasks start running their main loops. In the main loop each task prints task 
number and number of elapsed ticks since the last print at random interval that is between 1 – 2 seconds. 
Task 4 is debug print task. Tasks 1 – 3 must run at higher priority than the debug task. The tasks must use 
the debug function described above for printing.
