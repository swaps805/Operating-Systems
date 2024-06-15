// MAIN OBJECTIVE 
// The main aim of the program is to check for tetrahedral numbers in the range of 1 to N. 
// It is achieved by a multi-process approach. Here the parent process creates 
// a shared memory buffer that can be accessed by  K child processes.
// Parent buffer is populated with numbers from 1 to N.
// The child processes create their own local shared memory buffers.
// The child process determines if a number is tetrahedral or not 
// based on the data stored in Parent shared memeory buffer (each child process examines specific indexes)
// These local child buffers are shared with the main Parent process that consolidates the search results.


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <math.h>


// Function to check for tetrahedral numbers
int is_tetrahedral(int n) {
    // Take cuberoot and round it to nearest integer and check
    int i = (int)cbrt(n * 6);
    return (i * (i + 1) * (i + 2) == n * 6);
}

// Consolidating the work of Child process in Parent process
void print_output(int K,char C_NAME[K][50], int CHILD_BUFFER){
    // Setting up ptr and fd to access child process's local buffer
    int *C_ptr;
    int C_fd;
    int C_SIZE = sizeof(int) * CHILD_BUFFER;
    FILE *fptr = fopen("OutMain.log", "w");

    // Opening shared memory segment seperately for K child process
    for (int i = 0; i < K; i++) {

        // access child  buffers
        C_fd = shm_open(C_NAME[i], O_RDWR, 0666);
        C_ptr = (int *)mmap(0, C_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, C_fd, 0);
        

        fprintf(fptr, "P%d : ", i + 1);
        for (int j = 0; j < CHILD_BUFFER; j++) {
            // if the value of buffer at the given index is not 0
            // it is a tetrahedral number and print it on the log file
            if (C_ptr[j]) {
                fprintf(fptr, "num%d ", C_ptr[j]);
            }
        }
        printf("Child %d Complete \n", i + 1);
        fprintf(fptr, "\n");
        
        // relinquish memory resource 
        munmap(C_ptr, C_SIZE); // unmap the pointers to memory
        close(C_fd); // close the file descriptors of the child processes
        shm_unlink(C_NAME[i]); // unlink the maped memory space
    }

    fclose(fptr);

}

int main(int argc, char *argv[]) {
    int N, K;
    FILE *fptr;
    char fname[200]; // filenames for OutFile for different process
    
    // using command line arguments to input N and K values from input file
    if (argc != 2) {
        printf("Please enter an input text file");
        return 1;
    } else {
        fptr = fopen(argv[1], "r");
        fscanf(fptr, "%d %d", &N, &K);
    }
    fclose(fptr);


    struct timeval start, end;
    

    // Parent Process
    // Create a Buffer of size N and populates the buffer with numbers from 1 to N
    // Each Child process copies  specific indexes of the buffer onto its local buffer to check for 
    // tetrahedral numbers.

    int PARENT_BUFFER = N;
    int P_SIZE = sizeof(int) * PARENT_BUFFER; // reserve appropriate bytes for memory

    char *P_NAME = "/SWAPPY"; // parent process name '/SWAPPY'
    int P_fd; // Parent file Descriptor
    int *P_ptr; // Parent shared memeory pointer

        P_fd = shm_open(P_NAME, O_CREAT | O_RDWR, 0666); // open shared memory segment with appropriate permissions
        ftruncate(P_fd, P_SIZE);// truncate the size of the segment to predefined value
        P_ptr = (int *)mmap(0, P_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, P_fd, 0); // obtain a pointer to the mapped segment

    // populate parent buffer from 1 to N
    for (int i = 0; i < PARENT_BUFFER; i++) {
        P_ptr[i] = 1 + i;
    }

    // start measuring time for child process
    gettimeofday(&start, NULL);

    
    int *C_ptr; // Child file pointer to shared memory
    int C_fd; // Child file Descriptor
    char C_NAME[K][50]; 

    // creating names for differnt child process
    // Names are :
    // /Process1 ,/Process2, ....., /Processi
    for (int i = 0; i < K; i++) {
        sprintf(C_NAME[i], "/Process%d", i + 1);
    }
    
    // size of child buffers equally divided among process
    int CHILD_BUFFER = (N / K) + 1;
    int C_SIZE = sizeof(int) * CHILD_BUFFER;



    // creating k child processes
    for (int i = 0; i < K; i++) {

        // print to Outfile
        sprintf(fname, "OutFile%d.log", i + 1);

        pid_t pid = fork();

        // if fork is unsucessful then exit
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        // if the process is a child process then create a shared memeory segment
        if (pid == 0) {
            fptr = fopen(fname, "w");


            C_fd = shm_open(C_NAME[i], O_RDWR | O_CREAT, 0666);
            ftruncate(C_fd, C_SIZE);
            C_ptr = (int *)mmap(0, C_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, C_fd, 0);


            // An index counter idx is set to 0, as and when a tetrahedral number 
            // is found, it stores it in the buffer and increments the index. 
            // The workload is distributed as follows with each process working on the following set of numbers
            // by acessing the parent buffer
            //      i.	 Process 1 :  1, k+1 , 2k+1 ...
            //      ii.	 Process 2 :  2, k+2 , 2k+2 ...
            //      iii. Process 3 :  3, k+3 , 2k +3 ...
            //      iv.	 Process k :  k , 2k ,3k ...


            int idx = 0;

            for (int j = i; j < PARENT_BUFFER; j += K) {
                if (j < PARENT_BUFFER && is_tetrahedral(P_ptr[j])) {
                    // child process checks through numbers provided by parent buffer(P_ptr[j])
                    // if tetrahedral, write to log file as well as store in shared buffer
                    fprintf(fptr, "%d : Is a tetrahedral number\n", P_ptr[j]);
                    C_ptr[idx] = P_ptr[j];
                    idx++; // increment index
                } else if (j < PARENT_BUFFER) {
                    // if not, do not store in buffer
                    fprintf(fptr, "%d : Not a tetrahedral number\n", P_ptr[j]);
                }
            }

            fclose(fptr);
            exit(EXIT_SUCCESS);
        }
    }

    // wait for each child process to complete
    for (int i = 0; i < K; i++) {
        wait(NULL);
    }

    // parent process consolidates the results
    print_output(K,C_NAME,CHILD_BUFFER);

    // stop recording time and calculate the total time elapsed
    gettimeofday(&end, NULL);
    double time_elapsed = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
    printf("The total time elapsed is: %.2f ms\n", 1000*time_elapsed);
    

    // relinquish memory from parent process
    munmap(P_ptr, P_SIZE);
    close(P_fd);
    shm_unlink(P_NAME);


    return 0;
}
