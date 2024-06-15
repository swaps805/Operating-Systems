// Objective: This assignment aims to perform parallel matrix multiplication through a Dynamic
// mechanism in C++.

#include <iostream>
#include <pthread.h>
#include <fstream>
#include <bits/stdc++.h>
#include <sys/time.h>
#include <atomic>
using namespace std;

#define MAX 4096 // the maximum size of input matrix 

// globally define empty matrix and initialise them later
int A[MAX][MAX]; // input matrix
int mat_atm[MAX][MAX]; // output matrices
int mat_CAS[MAX][MAX];
int mat_TAS[MAX][MAX];
int mat_bnd_CAS[MAX][MAX];



// argument passed to square functions
typedef struct _th_param_{
    int thread_id;
    int mat_size;
    int num_threads;
    int row_inc;
}th_param;


// TAS -------------------------------------------------------------------------
atomic_flag TAS_lock = ATOMIC_FLAG_INIT; // atomic flag
int count_TAS = 0; // counter
void* square_TAS(void* arg) {
    // dereference and obtains the thread parameters
    th_param* data = (th_param*)arg;
    int N = data->mat_size;
    int K = data->num_threads;
    int R_inc = data->row_inc;
    int id = data->thread_id;
    int start;

    while(true){

        while (TAS_lock.test_and_set(memory_order_acquire)); // Acquire the lock
        // if lock is false then acquire the lock and set to true, if true then in spinlock
        // memory_order_acquire ensures memory operations visible to all thraeds

        // critical section
        start = count_TAS;
        count_TAS += R_inc;

        // set the lock to false and modify the memory 
        TAS_lock.clear(memory_order_release); 
        if(start >= N) break; // matrix computation completed
        

        int end = start + R_inc; // determine end row and check its within bounds
        end = (end > N) ? N : end;


        // remainder section, compute matrix
        for (int i = start; i < end; i++) {
            for (int j = 0; j < N; j++) { 
                mat_TAS[i][j] = 0;
                for (int k = 0; k < N; k++) {
                    mat_TAS[i][j] += A[i][k] * A[k][j]; // compute C(i,j)
                }
            }
        }
    }
    return NULL;

}

// CAS -------------------------------------------------------------------------
atomic<int> CAS_lock = 0; // atomic lock that behaves as counter
void* square_CAS(void* arg) {
    // dereference and obtain the thread parameters
    th_param* data = (th_param*)arg;
    int N = data->mat_size;
    int K = data->num_threads;
    int R_inc = data->row_inc;
    int id = data->thread_id;


    while(true){
        // load the lock into start
        int start = CAS_lock.load();
        if(start >= N) break; // matrix computation completed 
        
        // acquire and update the start 
        while (!CAS_lock.compare_exchange_strong(start, start + R_inc)) {
            // if lock value =  start then return true  and increment lock (value of start stays the same)
            // break out of loop (lock changed from expected to desired value)
            // if false then the value of CAS_lock had been incremented by someother thread and is no longer
            //  equal to previous start, so it returns false and changes start to new incremented value of start
            // (expected value modified)
            start = CAS_lock.load();
        }
        // obtain the end value
        int end = start + R_inc;
        end = (end > N) ? N : end;
        
        // loop over the row indices within start to end
        for (int i = start; i < end; i++) {
            for (int j = 0; j < N; j++) { // loop over the corresponding coloumn
                mat_CAS[i][j] = 0;
                for (int k = 0; k < N; k++) {
                    mat_CAS[i][j] += A[i][k] * A[k][j]; // compute C(i,j)
                }
            }
        }
    }
    return NULL;

}

// Bounded CAS ---------------------------------------------------------------
atomic<int> CAS_bnd_lock = false; //initially lock is false
vector<bool> waiting(256, false); // waiting array set to false
int count_bndCas = 0; // counter
void* square_bnd_CAS(void* arg) {

    th_param* data = (th_param*)arg;
    int N = data->mat_size;
    int K = data->num_threads;
    int R_inc = data->row_inc;
    int id = data->thread_id;
    int expected = 0;
    int start, key, end;

    while (true) {
        waiting[id] = true; // thread i ready to run
        key = 0; // set key = 0

        // if lock is false, it sees that expected val is 0 it returns true and the thread 
        // enters its critical section, lock set to true
        // if lock is true, it sees that expected value doesnt match with true, 
        // it returns false  to key  and stays in spinlock
        while(waiting[id] && key == 0){
            key = CAS_bnd_lock.compare_exchange_strong(expected, 1);
        }
        waiting[id] = false;

        /* critical section */
        start = count_bndCas;
        if(start >= N) break;
        count_bndCas += R_inc;
        end = start + R_inc;

        // decide next thread to choose from
        int jd = (id + 1) % K;
        while (jd != id && !waiting[jd]) {
            jd = (jd + 1) % K;
        }
        if (jd == id) {
            CAS_bnd_lock = false;
        } 
        else {
            waiting[jd] = false;
        }

        // remainder section compute matrix
        end = (end > N) ? N : end;
        for (int i = start; i < end; i++) {
            for (int j = 0; j < N; j++) { // loop over the corresponding coloumn
                mat_bnd_CAS[i][j] = 0;
                for (int k = 0; k < N; k++) {
                    mat_bnd_CAS[i][j] += A[i][k] * A[k][j]; // compute C(i,j)
                }
            }
        }

    }
    
    return NULL;

}
// Atomic -------------------------------------------------------------------------
atomic<int> count_atm = 0; // atomic counter
void* square_atomic(void* arg) {
    // dereference and obtain the thread parameters
    th_param* data = (th_param*)arg;
    int N = data->mat_size;
    int K = data->num_threads;
    int R_inc = data->row_inc;
    int id = data->thread_id;
     
    while(true){
        int start = count_atm.fetch_add(R_inc);  // load the counter into start variable and atomically increment
        if(start >= N) break; // matrix computation completed 

        int end = (start + R_inc); // find end range 
        end = (end > N) ? (N) : end;
        // loop over the row indices within start to end
        for (int i = start; i < end; i++) {
            for (int j = 0; j < N; j++) { // loop over the corresponding coloumn
                mat_atm[i][j] = 0;
                for (int k = 0; k < N; k++) {
                    mat_atm[i][j] += A[i][k] * A[k][j]; // compute C(i,j)
                }
            }
        }
    }
    
    return NULL;

}


// print output in text file
void print_output(int n, double t1, double t2, double t3, double t4){
    ofstream outfile("out.txt"); //write to out.txt
    outfile<<"time taken by TAS is "<< t1 <<"s\n\n";
    outfile<<"time taken by CAS is "<< t2 <<"s\n\n";
    outfile<<"time taken by Bounded CAS is "<< t3 <<"s\n\n";
    outfile<<"time taken by Atomic is "<< t4 <<"s\n\n";
    
    outfile<<"\nTAS Lock \n\n";
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n;j++){
            outfile << mat_TAS[i][j] << " ";
        }
        outfile<<"\n";
    }

    outfile<<"\nCAS lock \n\n";
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n;j++){
            outfile << mat_CAS[i][j] << " ";
        }
        outfile<<"\n";
    }
    
    outfile<<"\nBounded CAS Lock \n\n";
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n;j++){
            outfile << mat_bnd_CAS[i][j] << " ";
        }
        outfile<<"\n";
    }

    outfile<<"\nAtomic Lock \n\n";
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n;j++){
            outfile << mat_atm[i][j] << " ";
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
    int N, K, rowInc;
    infile >> N >> K >> rowInc;

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
    //------------------------------------------------------------------------
    pthread_t threads_1[K],threads_2[K],threads_3[K],threads_4[K]; // thread runner
    th_param data[K]; // thread arguments passed to function
    struct timeval start, end;


    for (int i = 0; i < K; ++i) {
        data[i].thread_id = i;
        data[i].mat_size = N;
        data[i].num_threads = K;
        data[i].row_inc = rowInc;
        
    }
    //-------------------------------------------------------------------------
    // TAS
    gettimeofday(&start, NULL);
    for (int i = 0; i < K; i++) {
        pthread_create(&threads_1[i], NULL, square_TAS, &data[i]);
    }
    // After the thread has finished its task, join the threads and consolidate results
    for (int i = 0; i < K; i++) {
        pthread_join(threads_1[i], NULL);
    }
    gettimeofday(&end, NULL); // record end time
    double time_TAS = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
     
    //------------------------------------------------------------------------
    // CAS
    gettimeofday(&start, NULL);
    for (int i = 0; i < K; i++) {
        pthread_create(&threads_2[i], NULL, square_CAS, &data[i]);
    }
    // After the thread has finished its task, join the threads and consolidate results
    for (int i = 0; i < K; i++) {
        pthread_join(threads_2[i], NULL);
    }
    gettimeofday(&end, NULL); // record end time
    double time_CAS = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
     
    
    //-----------------------------------------------------------------------
    // Bounded CAS
    gettimeofday(&start, NULL);
    for (int i = 0; i < K; i++) {
        pthread_create(&threads_3[i], NULL, square_bnd_CAS, &data[i]);
    }
    // After the thread has finished its task, join the threads and consolidate results
    for (int i = 0; i < K; i++) {
        pthread_join(threads_3[i], NULL);
    }

    gettimeofday(&end, NULL); // record end time
    double time_bnd_CAS = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
    //-----------------------------------------------------------------------
    // Atomic
    gettimeofday(&start, NULL);
    for (int i = 0; i < K; i++) {
        pthread_create(&threads_4[i], NULL, square_atomic, &data[i]);
    }
    // After the thread has finished its task, join the threads and consolidate results
    for (int i = 0; i < K; i++) {
        pthread_join(threads_4[i], NULL);
    }
    gettimeofday(&end, NULL); // record end time
    double time_atm = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
    //-----------------------------------------------------------------------
     
    print_output(N, time_TAS, time_CAS, time_bnd_CAS, time_atm);
    cout<< "printed to file";

}