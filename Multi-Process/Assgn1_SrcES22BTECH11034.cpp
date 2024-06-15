// ES22BTECH11034
// Main Objective: Develop a multithreaded program to computes the square of a matrix using POSIX threads
// The program creates K threads that works on different rows of the input matrix
// Chunk method: it creates chunks of size N/K and each thread works on its assigned contiguos chunks
// Mixed method: each thread works on rows in incremental values of K
// Diagonal method: this method splits the work among threads by assigning a thread to a diagonal of a matrix (topleft to bottomright)
//                  A given thread selects a particular diagonal and  increments in steps of K
// It is implemented using POSIX threads

#include <iostream>
#include <pthread.h>
#include <fstream>
#include <bits/stdc++.h>
#include <sys/time.h>

using namespace std;

#define MAX 4096 // the maximum size of input matrix 

// globally define empty matrix and initialise them later
int A[MAX][MAX]; // input matrix
int C_chunk[MAX][MAX]; // result matrices
int C_mixed[MAX][MAX];
int C_diagonal[MAX][MAX];

// thread parameter structure that contains information on thread number (i), N, K, p (size)
typedef struct th_param{
    int matrix_size;
    int num;
    int size;
    int max_thread;
}th_param;


// chunk method
void* square_chunk(void* arg) {
    th_param* data = (th_param*)arg; // typecasting to th_param structure
    int start = data->num * data->size;  //start index = i*p
    int end = (data->num + 1) * data->size; // end index = (i+1)*p 
    int N = data->matrix_size; 
    
    // if its the last thread assign it the last remaining rows
    if (data->num == data->max_thread - 1){
        end  = N;
    }

    // loop over the row indices within start to end
    for (int i = start; i < end; i++) {
        for (int j = 0; j < N; j++) { // loop over the corresponding coloumn
            C_chunk[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C_chunk[i][j] += A[i][k] * A[k][j]; // compute C(i,j)
            }
        }
    }
    return NULL;

}

// mixed method
void* square_mixed(void *arg){
    th_param* data = (th_param*)arg; // typecasting to th_param structure
    int start = data->num; // i value
    int step = data->max_thread; // K
    int N = data->matrix_size;  // N

    // loop over the row indices within start to N in steps of K
    for (int i = start; i < N; i += step) {
        for (int j = 0; j < N; j++) { // loop over the corresponding coloumn
            C_mixed[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C_mixed[i][j] += A[i][k] * A[k][j]; // compute C(i,j)
            }
        }
    }
    return NULL;
}

// bonus method 
// this method splits the work among threads by assigning a thread to a diagonal of a matrix(topleft to bottomright)
// A given thread selects a particular diagonal and  increments in steps of K
void* square_diagonal(void *arg) {
    th_param* data = (th_param*)arg;
    int i = data->num;
    int p = data->size;
    int N = data->matrix_size;
    int K = data->max_thread;
    
    // the diagonal is identified using  id =  x_coord - y_coord
    // id>0 means lower triangular half, if id<0 upper triangular half

    // diag is the diagonal id ranges from (N-1) to -(N-1); in total 2N-1 diagonals
    // N-1-i denotes from where the ith thread to start execution in steps of K
    for (int diag = (N - 1) - i; diag >= -(N - 1); diag -= K) {
        if (diag >= 0) {// lower triangular half diag value is always +ve as x_coord > y_coord
            int x_coord = N - 1; // always starts at N-1
            int y_coord = N - 1 - diag; // since x_coor- y_coor = diag
            
            for (int j = y_coord; j >= 0; j--) { // decrement y-coordinte till it reaches 0
                C_diagonal[x_coord][j] = 0;  
                for (int k = 0; k < N; k++) {//compute the diagonal values of C-diagonal
                    C_diagonal[x_coord][j] += A[x_coord][k] * A[k][j];
                }
                x_coord--; //decrement x values to go top left in the diagonal
            }
        } else {
            // lower triangular half diag value is always -ve as x_coord < y_coord
            int x_coord = N - 1 + diag;// since x_coor- y_coor = diag but here diag is -ve so added
            int y_coord = N - 1;
            for (int j = x_coord; j >= 0; j--) { // decrement x-coordinte till it reaches 0
                C_diagonal[j][y_coord] = 0;  
                for (int k = 0; k < N; k++) {
                    C_diagonal[j][y_coord] += A[j][k] * A[k][y_coord];
                }
                y_coord--;//decrement y values to go top left in the diagonal
            }
        }
    }
    return NULL;
}


// function to print to outputfile
void print_output(int n, int arr1[][MAX], int arr2[][MAX],int arr3[][MAX],double t1, double t2, double t3){
    ofstream outfile("out.txt"); //write to out.txt
    outfile<<"time taken by chunk square is "<< t1 <<"s\n\n";
    outfile<<"time taken by mixed square is "<< t2 <<"s\n\n";
    outfile<<"time taken by diagonal square is "<< t3 <<"s\n\n";

    outfile<<"\nChunk method \n\n";
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n;j++){
            outfile << arr1[i][j] << " ";
        }
        outfile<<"\n";
    }
    
    outfile<<"\nMixed method \n\n";
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n;j++){
            outfile << arr2[i][j] << " ";
        }
        outfile<<"\n";
    }

    outfile<<"\nDiagonal method \n\n";
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n;j++){
            outfile << arr3[i][j] << " ";
        }
        outfile<<"\n";
    }
    
    outfile.close();
}



int main(int argc,char *argv[]){
    // obtain input file as command line argument
    if (argc != 2) {
        cout << "please provide input file";
        return 1;
    }

    ifstream infile(argv[1]);

    if (!infile.is_open()) {
        cerr << "Unable to open input file: " << argv[1] << "\n";
        return 0;
    }
    int N, K;
    infile >> N >> K;

    // check if number of threads is less than size of matrix
    if(N < K){
        cout<<"The size of matrix should be greater than number of threads";
        return 0;
    }
    
    // read the input matrix from input file
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (!(infile >> A[i][j])) {
                cerr << "Error reading matrix from file.\n";
                infile.close();
                return 1;
            }
        }
    }

    infile.close();
    //--------------------------------------------------------------------

    pthread_t threads_1[K], threads_2[K], threads_3[K]; // worker threads
    int p = N/K; // chunk size
    th_param data[K]; // array of th_param structure to store thread information which will be passed as function arg

    struct timeval start, end; 

    // initialise the data of input argument of the square function
    for (int i = 0; i < K; ++i) {
        data[i].matrix_size = N;
        data[i].num = i;
        data[i].size = p;
        data[i].max_thread = K;    
    }
    
    //--------------------------------------------------------------------
    // Chunk Method

    gettimeofday(&start, NULL); // record start time
    // create K threads and pass the thread parameters to the corresponding function
    // the thread attribute is set to NULL
    for (int i = 0; i < K; i++) {
        pthread_create(&threads_1[i], NULL, square_chunk, &data[i]);
    }

    // After the thread has finished its task, join the threads and consolidate results
    for (int i = 0; i < K; i++) {
        pthread_join(threads_1[i], NULL);
    }

    gettimeofday(&end, NULL); // record end time
    // calulate time elapsed
    double time_elapsed_chunk = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
    
    //-------------------------------------------------------------------- 
    // Mixed Method

    gettimeofday(&start, NULL); // record start time
    // create K threads and pass the thread parameters to the corresponding function
    // the thread attribute is set to NULL
    for (int i = 0; i < K; i++) {
        pthread_create(&threads_2[i], NULL, square_mixed, &data[i]);
    }
    // After the thread has finished its task, join the threads and consolidate results
    for (int i = 0; i < K; i++) {
        pthread_join(threads_2[i], NULL);
    }

    gettimeofday(&end, NULL); // record end time
    // calulate time elapsed
    double time_elapsed_mixed = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;


    //--------------------------------------------------------------------------------------------
    // diagonal method
    
    gettimeofday(&start, NULL);
    for (int i = 0; i < K; i++) {
        pthread_create(&threads_3[i], NULL, square_diagonal, &data[i]);
    }

    for (int i = 0; i < K; i++) {
        pthread_join(threads_3[i], NULL);
    }

    gettimeofday(&end, NULL);
    double time_elapsed_diagonal = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;


    //----------------------------------------------------------------------------------------

    cout<<"printed to file";

    // print to out.txt
    print_output(N, C_chunk, C_mixed, C_diagonal,time_elapsed_chunk, time_elapsed_mixed, time_elapsed_diagonal);
    return 0;

}

