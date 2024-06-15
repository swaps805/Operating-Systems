// ES22BTECH11034
// Experiment 2
// Main Objective: To measure the performance benefits of binding a set of thread to a given core
// K/2 threads are bounded to C/2 cores and the the rest are scheduled by the OS
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

// result in output file
void print_output(int n, int arr1[][MAX],int arr2[][MAX], double t1, double t2, double t3, double t4, double t5, double t6){
    ofstream outfile("out.txt"); //write to out.txt
    outfile<<"avg total time taken by chunk square  is "<< t1 <<"s\n\n";
    outfile<<"avg time taken by chunk square bounded thread is "<< t2 <<"s\n\n";
    outfile<<"avg time taken by chunk square normal thread is "<< t3 <<"s\n\n";
    outfile<<"avg total time taken by chunk mixed  is "<< t4 <<"s\n\n";
    outfile<<"avg time taken by mixed square bounded thread is "<< t5 <<"s\n\n";
    outfile<<"avg time taken by mixed square normal thread is "<< t6 <<"s\n\n";

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
    if(N < K  || BT > K){
        cout<<"ensure that N >= K and BT <= K";
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

    pthread_t threads_1[K], threads_2[K], threads_3[K], threads_4[K];
    th_param data[K]; //  argument passed to matrix calculating function to determine where to start execution
    cpu_set_t cpuset; // init cpu_set_t data structure

    int cores = (C / 2); // no. of core to bind threads to
    int th_per_core = BT / cores; // no. of threads per core
    int p = N/K; // chunk
    struct timeval start, end;

    // inititalise parameters to be passed to threads
    for (int i = 0; i < K; ++i) {
        data[i].matrix_size = N;
        data[i].num = i;
        data[i].size = p;
        data[i].max_thread = K;    
    }
    //-------------------------------------------------------------------------------

    //Chunk method  assignement 1

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

    gettimeofday(&end, NULL);
    double time_chunk = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;

    //------------ bound and unbound threads---------------------------------

    gettimeofday(&start, NULL); // start time for bounded threads

    if(th_per_core == 0){ // if threads to be bound < C/2, bind to single core
        CPU_ZERO(&cpuset); // clear the cpu_set structure before initilising
        CPU_SET(0, &cpuset); // bind the thraeds to core0
        for(int i = 0; i < BT; i++){
            // create threads and bind them
            pthread_create(&threads_2[i], NULL, square_chunk, &data[i]);
            pthread_t tid = threads_2[i]; // get tid
            if (pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset) == -1) {// bind the thread to the core
                perror("sched_setaffinity");
                return 1;
            } 
        }
        // join the bounded threads
        for(int i = 0; i < BT; i++){
            pthread_join(threads_2[i], NULL);
        }
 
    }
    else{
        // the number of threads is more that C/2
        for(int i = 0; i < cores ; i++){ // iterate over no. of cores
            CPU_ZERO(&cpuset); // clear the cpu_set set structure before initialising
            CPU_SET(i, &cpuset); // bind the threads  BT/(C/2)

            for(int j = 0; j < th_per_core; j++){ // iterate over threads to be given to a core
                pthread_create(&threads_2[i * th_per_core + j], NULL, square_chunk, &data[i * th_per_core + j]); // create the thread for chunkmethod
            }

            for(int j = 0; j < th_per_core; j++){
                pthread_t tid = threads_2[i * th_per_core + j];
                if (pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset) == -1) {
                    perror("sched_setaffinity");
                    return 1;
                }
            }
        }
        // join the bounded threads
        for(int i = 0; i < BT; i++){
            pthread_join(threads_2[i], NULL);
        }
        
    }
    // time for completeion of bounded threads
    gettimeofday(&end, NULL); 
    double time_chunk_BT = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;

    // unbounded threads (K/2) assigned to remaining cores
    gettimeofday(&start, NULL); // start time for unbounded threads
    for (int i = BT; i < K; i++) { 
        pthread_create(&threads_2[i], NULL, square_chunk, &data[i]);
    }

    for (int i = BT; i < K; i++) {
        pthread_join(threads_2[i], NULL);
    }
    // end time for unbounded threads
    gettimeofday(&end, NULL);
    double time_chunk_NT = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
    //------------------------------------------------------------------------------------------
    
    // mixed method
    // same loop structure as chunk method assignmet 1

    gettimeofday(&start, NULL); 

    for (int i = 0; i < K; i++) {
        pthread_create(&threads_3[i], NULL, square_mixed, &data[i]);
    }
    for (int i = 0; i < K; i++) {
        pthread_join(threads_3[i], NULL);
    }

    gettimeofday(&end, NULL); 
    double time_mixed = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;

    //---------------------- bound and unbound threads-----------------

    gettimeofday(&start, NULL);
    if(th_per_core == 0){ 
        CPU_ZERO(&cpuset);
        CPU_SET(0, &cpuset);
        for(int i = 0; i< BT; i++){
            pthread_create(&threads_4[i], NULL, square_mixed, &data[i]);
            pthread_t tid = threads_4[i];
            if (pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset) == -1) {
                perror("sched_setaffinity");
                return 1;
            } 
        }
        for(int i = 0; i < BT; i++){
            pthread_join(threads_4[i], NULL);
        }

    }
    else{
        for(int i = 0; i < cores ; i++){
            CPU_ZERO(&cpuset);
            CPU_SET(i, &cpuset);

            for(int j = 0; j < th_per_core; j++){
                pthread_create(&threads_4[i * th_per_core + j], NULL, square_mixed, &data[i * th_per_core + j]); // create the thread for chunkmethod
            }

            for(int j = 0; j < th_per_core; j++){
                pthread_t tid = threads_4[i * th_per_core + j];
                if (pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset) == -1) {
                    perror("sched_setaffinity");
                    return 1;
                }
            }
        }
        for(int i = 0; i < BT; i++){
            pthread_join(threads_4[i], NULL);
        }
        
    }
    gettimeofday(&end, NULL);
    double time_mixed_BT = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;

    gettimeofday(&start, NULL);
    
    for (int i = BT; i < K; i++) {
        pthread_create(&threads_4[i], NULL, square_mixed, &data[i]);
    }

    for (int i = BT; i < K; i++) {
        pthread_join(threads_4[i], NULL);
    }

    gettimeofday(&end, NULL);
    double time_mixed_NT = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
    
    //----------------------------------------------------------------------

    // average time for each thread in assignment1, bounded and normal state
    cout<< "printed to outputfile";
    print_output(N, C_chunk, C_mixed, time_chunk/K , time_chunk_BT/(BT), time_chunk_NT/(K - BT), time_mixed/K, time_mixed_BT/(BT), time_mixed_NT/(K-BT));
    return 0;

}

