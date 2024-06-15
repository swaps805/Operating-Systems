// Fair reader writer solution using semaphores and thread library in cpp
#include <bits/stdc++.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>
#include <semaphore.h>
#include <fstream>
#include <iomanip> 
#include <sys/time.h>

using namespace std;

// structure passed as thread argument
typedef struct _th_param_ {
    int th_id;
    int k_reader;
    int k_writer;
    int cs_time;
    int rem_time;
} th_param;

int count_readers = 0;
int count_writers = 0;
sem_t r_lock,  resource, print_l, serv_q;
// r_lock reader lock
// resouce lock to CS
// ser_q lock to queue the requests to CS
// print_l to synchonise printing to log file

vector<vector<double>> read_data(100, vector<double>(100));
vector<vector<double>> write_data(100, vector<double>(100));

ofstream outfile_1;

// get the current time
time_t getSysTime() {
    return chrono::system_clock::to_time_t(chrono::system_clock::now());
}

// convert to the current time to a format of HH:MM:SS:microsecond for better precision
string formatTime(time_t t) {
    stringstream ss;
    struct tm* tm_info;
    char buffer[80];

    tm_info = localtime(&t);
    strftime(buffer, 80, "%H:%M:%S", tm_info);
    ss << buffer << "." << setfill('0') << setw(6) << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() % 1000000;


    return ss.str();
}

void* reader(void* arg) {
    th_param* data = (th_param*)arg;
    int id = data->th_id;
    int kr = data->k_reader;
    int randCSTime = data->cs_time;
    int randRemTime = data->rem_time;
    struct timeval start, end;

    // generate random sleep times from an exponential distribution 
    default_random_engine generator;
    exponential_distribution<double> rem_distr(1/(randRemTime));
    exponential_distribution<double> CS_distr(1/(randCSTime));
    // serv_q is like a shared lock that needs to be acquired by both readers 
    // writers, this request for turn in a way ensures almost FIFO ordering
    // and hence fairness

    for (int i = 1; i <= kr; i++) {

        // Request entry to CS
        sem_wait(&print_l);
        time_t reqTime = getSysTime();
        gettimeofday(&start, NULL);// record the time it takes from request to entry
        outfile_1 << i << "th CS request by Reader Thread " << id << " at " << formatTime(reqTime) << endl;
        sem_post(&print_l);

        // Entry to CS, acquire r_lock to increment reader count
        sem_wait(&serv_q);
        sem_wait(&r_lock);
        count_readers++;
        if (count_readers == 1) {
            // if there is any reader then block it
            sem_wait(&resource);
        }
        sem_post(&serv_q);
        sem_post(&r_lock);

        // Critiacl section
        // Entry by reader thread
        sem_wait(&print_l);
        time_t enterTime = getSysTime();
        outfile_1 << i << "th CS Entry by Reader Thread " << id << " at " << formatTime(enterTime) << endl;
        gettimeofday(&end, NULL); // record end time
        double r_time = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
        read_data[id-1][i-1] = r_time;
        sem_post(&print_l);

         // perform reading by sleeping for certain time
        this_thread::sleep_for(chrono::milliseconds(static_cast<int>(CS_distr(generator))));

         // thread exits and decrement read_count
        sem_wait(&r_lock);
        count_readers--;
        if (count_readers == 0) {
            sem_post(&resource);
        }
        sem_post(&r_lock);

        // exit section 
        sem_wait(&print_l);
        time_t exitTime = getSysTime();
        outfile_1 << i << "th CS Exit by Reader Thread " << id << " at " << formatTime(exitTime) << endl;
        sem_post(&print_l);

        // simulate exit of a thread
        this_thread::sleep_for(chrono::milliseconds(static_cast<int>(rem_distr(generator))));
    }
    return NULL;
}

// writer thread
void* writer(void* arg) {
    // deference thread  argument
    th_param* data = (th_param*)arg;
    int id = data->th_id;
    int kw = data->k_writer;
    int randCSTime = data->cs_time;
    int randRemTime = data->rem_time;
    struct timeval start, end;

    // generate random sleep times from an exponentlia distribution with given mean
    default_random_engine generator;
    exponential_distribution<double> rem_distr(1/(randRemTime));
    exponential_distribution<double> CS_distr(1/(randCSTime));

    for (int i = 1; i <= kw; i++) {
        // Request entry to CS
        sem_wait(&print_l);
        time_t reqTime = getSysTime();
        gettimeofday(&start,NULL);
        outfile_1 << i << "th CS request by Writer Thread " << id << " at " << formatTime(reqTime) << endl;
        sem_post(&print_l);

        // acquire resource
        sem_wait(&serv_q);
        sem_wait(&resource);
        sem_post(&serv_q);

        // Critiacl section
        // Entry by writer thread
        sem_wait(&print_l);
        time_t enterTime = getSysTime();
        outfile_1 << i << "th CS Entry by Writer Thread " << id << " at " << formatTime(enterTime) << endl;
        gettimeofday(&end,NULL);
        double w_time = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
        write_data[id-1][i-1] = w_time;
        sem_post(&print_l);

        this_thread::sleep_for(chrono::milliseconds(static_cast<int>(CS_distr(generator))));
        sem_post(&resource);

        // simulate exit of thread
        sem_wait(&print_l);
        time_t exitTime = getSysTime();
        outfile_1 << i << "th CS Exit by Writer Thread " << id << " at " << formatTime(exitTime) << endl;
        sem_post(&print_l);

        this_thread::sleep_for(chrono::milliseconds(static_cast<int>(rem_distr(generator))));
    }

    return NULL;
}

//print to output file and print avg and max time
void print_time(int nr, int kr, int nw, int kw){
    ofstream outfile("FairRW_Average_time.txt,");
    double s_read = 0;
    double s_write = 0;
    double max_read = 0;
    double max_write = 0;

    outfile<< "Time taken by each reader thread to gain entry kr times" <<"\n\n";
    for(int i = 0; i < nr; i++){
        for(int j = 0; j < kr;j++){
            outfile << read_data[i][j] << " ";
            s_read += read_data[i][j];
            max_read = max(max_read, read_data[i][j]);
        }
        outfile<<"\n";
    }
    outfile<< "\nTime taken by each writer thread to gain entry kr times" <<"\n\n";
    for(int i = 0; i < nw; i++){
        for(int j = 0; j < kw;j++){
            outfile << write_data[i][j] << " ";
            s_write += write_data[i][j];
            max_write = max(max_write, write_data[i][j]);
        }
        outfile<<"\n";
    }
    outfile<< "\nTime in ms (fair) ";
    outfile<< "\nreaders time  " << s_read * 1000/(kr*nr);
    outfile<< "\nmax read time  " << max_read * 1000;
    outfile<< "\nwriters time  " << s_write * 1000/(kw*nw);
    outfile<< "\nmax write time  " << max_write * 1000;

    outfile.close();
}


int main(int argc, char* argv[]) {
    int nw;
    int nr;
    int kw;
    int kr;
    double randRemTime;
    double randCSTime;

    // obtain in the inputs as command line arguments
    if (argc != 2) {
        cout << "please provide input file";
        return 1;
    }
    ifstream infile(argv[1]);
    if (!infile.is_open()) {
        cerr << "Unable to open input file: " << argv[1] << "\n";
        return 0;
    }
    infile >> nw >> nr >> kw >> kr >> randCSTime >> randRemTime;

    // initialise all semaphores to 1
    sem_init(&r_lock, 0, 1);
    sem_init(&serv_q, 0, 1);
    sem_init(&resource, 0, 1);
    sem_init(&print_l, 0, 1);

    read_data.resize(nr, vector<double>(kr));
    write_data.resize(nw, vector<double>(kw));

    outfile_1.open("FairRW-log.txt");

    thread th_readers[nr];
    th_param reader_params[nr];
    // create the reader threads with their input arguments
    for (int i = 0; i < nr; i++) {
        reader_params[i].th_id = (i + 1);
        reader_params[i].k_reader = kr;
        reader_params[i].cs_time = randCSTime;
        reader_params[i].rem_time = randRemTime;

        th_readers[i] = thread(reader, &reader_params[i]);
    }
    // ----------------------------------------------------------------  

    thread th_writers[nw];
    th_param writer_params[nw];
    // create the writer threads with their input arguments
    for (int i = 0; i < nw; i++) {
        writer_params[i].th_id = (i + 1);
        writer_params[i].k_writer = kw;
        writer_params[i].cs_time = randCSTime;
        writer_params[i].rem_time = randRemTime;

        th_writers[i] = thread(writer, &writer_params[i]);
    }
    //---------------------------------------------------------------

    // join the threads
    for (int i = 0; i < nr; i++) {
        th_readers[i].join();
    }
    for (int i = 0; i < nw; i++) {
        th_writers[i].join();
    }

    sem_destroy(&print_l);
    sem_destroy(&r_lock);
    sem_destroy(&serv_q);
    sem_destroy(&resource);
    outfile_1.close();

    print_time(nr, kr, nw, kw);
    cout<<"printed";
    

    return 0;
}
