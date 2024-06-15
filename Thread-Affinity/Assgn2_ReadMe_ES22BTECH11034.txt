Main Objective

To evaluate the efficiency of a parallel matrix squaring algorithm by studying the impact
of assigning individual threads to CPU cores.
The question is divided experiment wise
program 1 : Exp1 : out of pb threads b threads are allocated to each core and rest K-pb threads are scheduled by OS
program 2 : Exp2 : K/2 threads are bound to C/2 cores and the rest are scheduled by OS

-------------------------------------------------------------------------------------------
Hardware:

Intel(R) Core(TM) i5-1021U CPU @ 1.60 Hz
cores : 4
logical cores : 8
-------------------------------------------------------------------------------------------
-> Compiling and building

Exp1 : "g++ Assgn1_Exp1_Src-ES22BTECH11034.cpp"
Exp2 : "g++ Assgn1_Exp2_Src-ES22BTECH11034.cpp"
--------------------------------------------------------------------------------------------
-> Execution 

"./a.out <inputfile>.txt" should be passed as command line argument
--------------------------------------------------------------------------------------------
-> Input text file 

C = 8
N >= K and K >= BT

Exp1 : K must be in multiple of C, b = K/C and BT must be multiple of b
Exp2 : K must be in multiple of C, K/2 cores must be divided to C/2 cores equally 

structure: example

4 4 2 2
1 0 0 1
7 8 9 0
0 0 1 0
0 9 9 9

-----------------------------------------------------------------------------------------------
-> Output

Exp1 : prints time taken by mixed and chunk method from creation to joining of threads
Exp2 : prints average time taken in assgn1, normal and normal for chunk and mixed
------------------------------------------------------------------------------------------------

Do note that the time taken highly depends the number of cores on the machine and how OS schedules the threads.
Hence the time taken can vary greatly especially for extemely large input.

------------------------------------------------------------------------------------------------