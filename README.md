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

### Part 2: 
Implement a program with five tasks and an event group. Task 4 is a watchdog task to monitor that tasks 1 – 3 run at least once every 30 seconds. Tasks 1 – 3 implement a loop that waits for button presses. When a 
button is pressed and released the task sets a bit in the event group, prints a debug message, and goes 
back to wait for another button press. Holding the button without releasing must block the main loop from 
running. Each task monitors one button. 

Task 4 prints “OK” and number of elapsed ticks from last “OK” when all (other) tasks have notified that they 
have run the loop. If some of the tasks does not run within 30 seconds Task 4 prints “Fail” and the number 
of the task(s) not meeting the deadline and then Task 4 suspends itself. 

Task 5 is the debug print task. It must run at a lower priority than tasks 1 – 4. 
