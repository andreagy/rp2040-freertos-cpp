# RP2040 FreeRTOS

### ARM Processors and Embedded Operating Systems course exercises
Each exercise is a branch of the original template file provided by the lecturer.

## Exercise 1: Queues
Implement a code lock that uses three tasks to read button presses and one task to process the button 
presses. The button presses from reader tasks must be passed to the processing task using a queue. The 
button to monitor and the queue to send to are passed through task parameters pointer. 
The processing task waits on the button queue with 5 second wait time. If a button press is received, then it 
is processed as shown in the state diagram. If a timeout occurs, then lock is returned to the initial state 
where it starts detecting the sequence from the beginning. An open lock is indicated by an led that is 
switched on for 5 seconds (=the time lock stays open). 
Make sure that stack for each task is large enough (512 as stack size parameter -> 2 kB) 
The sequence to open the lock is 0-0-2-1-2

![image](https://github.com/user-attachments/assets/336e2969-2e75-42d0-a4cf-bc7c80c6b170)
