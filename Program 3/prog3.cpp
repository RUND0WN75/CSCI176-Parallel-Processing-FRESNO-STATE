/*
Assignment abstract: an assignment demonstrating shared-memory programming using the Pthreads library. 
The main focus of this program is to compute various prefix sums using an array of prefix sum variables. 

Threads and semaphores are used to ensure calculations are correctly computed and thaat synchronization 
functions adequately. 

The program will output each thread ID based on the number of threads provided in the command line, followed by start and end values. 

After the creation/joining of threads with their respective execution times, various prefixSum
values are displayed based on various particular global array indices. 

Program can be executed with the following command: ./a.exe N  <----N = 1,2,4, or 8. 
*/
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <cstdlib>
#include <stdlib.h>
#include <ctime>
#include <iomanip>
using namespace std;
#define SIZE 500000017LL

//Global variables
int num_threads;
double *global; //For storing computed prefix sums
sem_t *semArr; //Dynamic array of semaphore pointers
pthread_mutex_t mutex1; //Atomic cout statement
void *prefixSum (void* rank);

//Slave function
void *prefixSum(void* rank) {
    int *myRank = (int*) rank;
    
    pthread_mutex_lock(&mutex1);
    printf("Thread ID: %d starting\n", *myRank);
    pthread_mutex_unlock(&mutex1);

    //myFirst & myLast index computations
    int div = SIZE / num_threads;
    int mod = SIZE % num_threads;
    int myFirst = *myRank * div;
    
    if (*myRank < mod && *myRank > 0) myFirst += *myRank;

    else if (*myRank > 0) myFirst += mod;

    int myLast = myFirst + div - 1;

    if (*myRank < mod) myLast++;

    pthread_mutex_lock(&mutex1);
    printf("Thread ID: %d > Start: %d, End: %d\n", *myRank, myFirst, myLast);
    pthread_mutex_unlock(&mutex1);
    double sum = 0.0;

    for (int i = myFirst; i <= myLast; i++) {
        sum += global[i];
        global[i] = sum;
    }

    //If at the first index, notify
    if (*myRank == 0) sem_post(&semArr[*myRank + 1]);
    
    //Else wait
    else { 
        sem_wait(&semArr[*myRank]);
        double prev = global[myFirst - 1];
        for (int i = myFirst; i <= myLast; i++) global[i] += prev;
        if (*myRank < num_threads - 1) sem_post(&semArr[*myRank + 1]);
    }

    //Closing semaphores
    sem_close(&semArr[*myRank]);

    return nullptr;
}

int main(int argc, char **argv) {
    //Obtaining total # of threads through command line
    printf("Starting program\n");
    num_threads = stoi(argv[1]);

    global = new double[SIZE];

    //Initializing all elements of array by i+1
    for (int i = 0; i < SIZE; i++) global[i] = (double)i+1.0;
    
    pthread_t *myThreads = new pthread_t[num_threads];
    pthread_mutex_init(&mutex1, NULL);
    
    int *ranks = new int[num_threads];
    semArr = new sem_t[num_threads]; 
    for (int i = 0; i < num_threads; i++) { 
        ranks[i] = i;
        sem_init(&semArr[i],0,0);
        semArr[i] = *(new sem_t());
    }
    
    //Creating threads
    printf("Starting threads\n");
    clock_t time = clock();
    for (int i = 0; i < num_threads; i++) pthread_create(&myThreads[i],NULL,prefixSum,(void*) &ranks[i]);
    
    printf("Thread creation finished\n");
    for (int i = 0; i < num_threads; i++) pthread_join(myThreads[i],NULL);
    
    printf("Threads joined\n");
    time = clock() - time;

    printf("%d threads took %.6f seconds\n", num_threads,(float)time/CLOCKS_PER_SEC);

    cout << fixed;
    cout << scientific;
    cout << "prefix_sum[0]: " <<  global[0] << endl;
    cout << "prefix_sum[1000000]: " << global[1000000] << endl;
    cout << "prefix_sum[10000000]: " << global[10000000] << endl;
    cout << "prefix_sum[100000000]: " << global[1000000] << endl;
    cout << "prefix_sum[200000000]: " << global[2000000] << endl;
    cout << "prefix_sum[300000000]: " << global[3000000] << endl;
    cout << "prefix_sum[400000000]: " << global[4000000] << endl;
    cout << "prefix_sum[500000016]: " << global[5000016] << endl;

    delete[] global;
    delete[] ranks;
    delete[] semArr;
}