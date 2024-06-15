Main Objective

This assignment aims to develop a solution for the reader writer problem
i) writers prefernce
ii) fair solution

-------------------------------------------------------------------------------------------

-> Compiling and building

"g++ rw-ES22BTECH11034.cpp"
"g++ frw-ES22BTECH11034.cpp"
--------------------------------------------------------------------------------------------
-> Execution 

"./a.out <inputfile>.txt" should be passed as command line argument
--------------------------------------------------------------------------------------------
-> Input text file 
nw, nr, kw, kr, RandCSTime,RandRemTime 
(ensure that nr,nw,kr,kw <= 100 )

RandCSTime and RandRemTime are the mean parameter to the exponential distribution
hence the lambda parameter passed is (1/mean) to the time generator

structure: example
10 10 5 5 10 5

-----------------------------------------------------------------------------------------------
-> Output

Log.txt: prints the output logs of reader and writers 
Average_time.txt: prints the avg and max time for request for each thread
------------------------------------------------------------------------------------------------

