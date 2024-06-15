// ES22BTECH11034
// Experiment 1
// Main Objective: To measure the performance benefits of binding a set of thread to a given core
// the number of bounded threads is incremented in steps of b (K/C) where in pb threads
// are bound to p cores and rest are scheduled by the OS
// It is implemented using POSIX threads using pthread_setaffinity_np()


#include <iostream>
#include <pthread.h>
#include <fstream>
#include <bits/stdc++.h>
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>
#include <stdio.h>

using namespace std;

#define MAX 4096

int A[MAX][MAX]; // input matrix
int C_chunk[MAX][MAX]; // result matrices
int C_mixed[MAX][MAX];

// th_param structure passed as argument to the matrix computation fn
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

// function to print thr output in out.txt
void print_output(int n, int arr1[][MAX],int arr2[][MAX], double t1, double t2){
    ofstream outfile("out.txt"); //write to out.txt
    outfile<<"time taken by chunk square  is "<< t1 <<"s\n\n";
    outfile<<"time taken by mixed square  is "<< t2 <<"s\n\n";
   

    outfile<<"\nChunk method \n\n";
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n;j++){
            outfile << arr1[i][j] << " ";
        }
        outfile<<"\n";
    }
    
    outfile<<"\nMixed method \n\n";
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            outfile << arr2[i][j] << " ";
        }
        outfile<<"\n";
    }

    outfile.close();
}



int main(int argc, char *argv[]){
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
    int N, K, C, BT;
    infile >> N >> K >> C >> BT;

    // check if number of threads is less than size of matrix
    if(N < K || BT > K){
        cout<<"please check input parameters: ensure N >= K and BT <= K";
        return 0;
    }
    
    // read the input matrix from input file
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (!(infile >> A[i][j])) {
                cerr << "Error reading matrix from file.\n";
                infile.close();
                return 1;
            }
        }
    }

    infile.close();


    // -----------------------------------------------------------------------------

    pthread_t threads_1[K], threads_2[K];
    th_param data[K]; // argument passed to matrix calculating function to determine where to start execution
    cpu_set_t cpuset; // init cpu_set_t data structure

    int b = K / C;  // no. of threads assigned on each core
    int cores = BT / b; // no. of cores to bind the threads
    int p = N / K; // chunk size
    struct timeval start, end;

    // inititalise parameters to be passed to threads
    for (int i = 0; i < K; ++i) {
        data[i].matrix_size = N;
        data[i].num = i;
        data[i].size = p;
        data[i].max_thread = K;    
    }
    //-------------------------------------------------------------------------------
    // Chunk method

    gettimeofday(&start, NULL); // start time
    for(int i = 0; i < cores ; i++){
        CPU_ZERO(&cpuset); // clear the CPU before initilising
        CPU_SET(i, &cpuset); // bind threadsds to core i

        for(int j = 0; j < b; j++){
            pthread_create(&threads_1[i * b + j], NULL, square_chunk, &data[i * b + j]); // create the thread for chunkmethod
        }

        for(int j = 0; j < b; j++){
            pthread_t tid = threads_1[i * b + j]; // get tid of created thread 
            if (pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset) == -1) { // set the affinity of the bound thread
                perror("sched_setaffinity"); // raise error 
                return 1;
            }

        }
    }
        
    // create the remaining threads to be scheduled by the OS
    for (int i = BT; i < K; i++) {
        pthread_create(&threads_1[i], NULL, square_chunk, &data[i]);
    }
    // consolidate results
    for (int i = 0; i < K; i++) {
        pthread_join(threads_1[i], NULL);
    }
    // measure the end time
    gettimeofday(&end, NULL);
    double time_chunk = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
    //------------------------------------------------------------------------------------------
    
    // Mixed method 
    // Same structure as Chunk method
    
    gettimeofday(&start, NULL);
    for(int i = 0; i < cores ; i++){
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);

        for(int j = 0; j < b; j++){
            pthread_create(&threads_2[i * b + j], NULL, square_mixed, &data[i * b + j]);
        }

        for(int j = 0; j < b; j++){
            pthread_t tid = threads_2[i * b + j];
            if (pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset) == -1) {
                perror("sched_setaffinity");
                return 1;
            }
        }
        
    }

        
    for (int i = BT; i < K; i++) {
        pthread_create(&threads_2[i], NULL, square_mixed, &data[i]);
    }

    for (int i = 0; i < K; i++) {
        pthread_join(threads_2[i], NULL);
    }

    gettimeofday(&end, NULL);
    double time_mixed = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
    
    //----------------------------------------------------------------------
    //print to out.txt
    cout<< "printed to outputfile";
    print_output(N, C_chunk, C_mixed, time_chunk, time_mixed);
    return 0;

}

