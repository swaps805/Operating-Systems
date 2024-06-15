Main Objective 

The main aim of the program is to check for tetrahedral numbers in the range of 1 to N. 
It is achieved by a multi-process approach. 
Here the parent process creates a shared memory buffer that can be accessed by the child processes. 
K separate child processes are created with their own local child buffers that determine 
if a number is tetrahedral or not from specific section of the parent buffer and stores them. 
These local child buffers are shared with the main Parent process that consolidates the search results.

---------------------------------------------------------------------------------------------------------
-> Compiling and building

"gcc Assgn1Src-ES22BTECH11034 -lm" -lm to link math.h header file

---------------------------------------------------------------------------------------------------------

-> Execution 

"./a.out <inputFile>.txt" the input file must be provided as a command line argument

---------------------------------------------------------------------------------------------------------

-> input text file
should consist of N and K in same line seperated by a space
"N K"
N: Number of numbers
K: Number of processes

---------------------------------------------------------------------------------------------------------

-> Output
1) Each process generates a OutFilei.log that determines which numbers are tetrahedral
2) The parent process creates OutMain.log file that logs which process found which tetrahedral number
3) "Child i complete " is printed on the console in newlines and also the total time 
   taken by the program to run in milliseconds (ms)

---------------------------------------------------------------------------------------------------------

-> program Structure

1) is_tetrahedral function():  This function returns true if the number is tetrahedral. 
   It takes the cube root of the integer n and rounds it to nearest integer and compares if it is tetrahedral.

2) Main function
    -Creates A parent shared memory buffer and populates it with numbers from 1 to N
    -The start time for the creation of Child process is recorded 
    -k child processes are created using fork() function call
     each child process sets up its own local memory and accesses the Parent shared memory buffer 
     to determine which numbers are tetrahedral and stores the results in its local buffer
    -The Parent waits for all child processes to complete

3) print_output() : The parent process acesses the child buffers and prints the identified numbers on the log file
   and appropriately reliquishes the memory from the child

4) The end time for program is recorded and parent shared buffer is closed

---------------------------------------------------------------------------------------------------------

->Note 

   The time elapsed highly depend on how the OS schedules the process and what background process
   are running. It also depend on how many cores are there on the machine and how the OS schedules the 
   processes to these cores. Hence the time measured may be quite different when run on different
   machine or OS

---------------------------------------------------------------------------------------------------------
