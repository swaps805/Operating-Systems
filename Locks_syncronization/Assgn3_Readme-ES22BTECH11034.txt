Main Objective

This assignment aims to perform parallel matrix multiplication through a Dynamic
mechanism in C++ using (all 4 methods in same program)
i)   test and set 
ii)  Compare and swap
iii) Bounded waiting with CAS
iv)  Atomic Increment

-------------------------------------------------------------------------------------------

-> Compiling and building

"g++ Assgn3_Src-ES22BTECH11034.cpp"
--------------------------------------------------------------------------------------------
-> Execution 

"./a.out <inputfile>.txt" should be passed as command line argument
--------------------------------------------------------------------------------------------
-> Input text file 


structure: example

4 4 2 
1 0 0 1
7 8 9 0
0 0 1 0
0 9 9 9

-----------------------------------------------------------------------------------------------
-> Output

out.txt 
prints time taken for all 4 lock techniques as well as result matrices
(the report has been made only till value N = 2048)
------------------------------------------------------------------------------------------------

Do note that the time taken highly depends the number of cores on the machine and how OS schedules the threads.
Hence the time taken can vary greatly especially for extemely large input.

------------------------------------------------------------------------------------------------