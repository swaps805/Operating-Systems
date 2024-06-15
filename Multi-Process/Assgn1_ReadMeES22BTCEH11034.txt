Main Objective

Develop a multithreaded program to compute the square of a matrix using 3 different approaches
It is implemented using POSIX threads
(All 3 methods are implemented in the 1 program)
-------------------------------------------------------------------------------------------
-> Compiling and building

"g++ Assgn1_SrcES22BTECH11034.cpp"
--------------------------------------------------------------------------------------------
-> Execution 

"./a.out <inputfile>.txt" should be passed as command line argument
--------------------------------------------------------------------------------------------
-> Input text file
should consist of N and K in same line seperated by a space
next line should be a matrix of size N*N
Max value of N is 4096
Please refer to the sample input file

structure: example

4 2
1 0 0 1
7 8 9 0
0 0 1 0
0 9 9 9
-----------------------------------------------------------------------------------------------
-> Output

the output is printed on out.txt
the time taken by all 3 methods
the matrices computed from these methods
------------------------------------------------------------------------------------------------

Do note that the time taken highly depends the number of cores on the machine and how OS schedules the threads.
Hence the time taken can vary greatly especially for extemely large input. If the number of cores is less,
time taken can be as large as 1-2 mins

------------------------------------------------------------------------------------------------

