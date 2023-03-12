/*
Assignment abstract: 
*/
#include <iostream>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <cstdlib>
#include <sstream>
using namespace std;
#define SIZE 500000017

//Global variables
int num_threads;
vector<double> global(SIZE); //For storing computed prefix sums
vector<string> msgArr; //Dynamic array of messages
vector<string> semName; //Dynamic array of semaphore names
sem_t **semArr; //Dynamic array of semaphore pointers
pthread_mutex_t mutex1; //Atomic cout statement

//Slave function
void *sendMsg(void *rank) {
    long my_rank = (long) rank;
    int dest = my_rank++;
    int source = (my_rank + num_threads - 1) % num_threads;

    //Doing type casting from int to string
    stringstream out1, out2;
    out1 << dest;
    out2 << my_rank;
    string destStr = out1.str();
    string my_rankStr = out2.str();

    string myMsg = "Hello to thread_" + destStr + " from thread" + my_rankStr;
    msgArr.push_back(myMsg); //Send msg to dest

    sem_post(semArr[dest]); //Notify to dest
    sem_wait(semArr[my_rank]); //Wait until source notifies me

    if (msgArr.at(my_rank) != "") {
        pthread_mutex_lock(&mutex1); //Make cout atomic
        cout << "Thread " << my_rank << " : " << msgArr.at(my_rank) << endl;
        pthread_mutex_unlock(&mutex1);
    }

    else {
        pthread_mutex_lock(&mutex1); //Make cout atomic
        cout << "Thread " << my_rank << " : " << "No message from thread " << source << endl;
        pthread_mutex_unlock(&mutex1);
    }

    return nullptr;
}

int main(int argc, char **argv) {
    //Initializing all elements of array by i+1
    for (int i = 0; i < global.size(); i++) global.at(i+1);

    long thread_id; //Long for type casting with void*
    //Obtaining total # of threads through command line
    //num_threads = atoi(argv[1]);

    pthread_t *myThreads = new pthread_t[num_threads];
    pthread_mutex_init(&mutex1, nullptr);

    //msgArr.push_back(num_threads);

    //Initializing array with empty strings
    for (thread_id = 0; thread_id < num_threads; thread_id++) msgArr.at(thread_id) = "";

    semArr = new sem_t*[num_threads];
    //semName.push_back(num_threads);

    //Initializing sem to 0 (locked), sem_open needs c_str type
    for (thread_id = 0; thread_id < num_threads; thread_id++) {
        semName.at(thread_id) = "sem " + thread_id;
        //semArr[thread_id] = sem_open(semName.at(thread_id).c_str(), 0_CREAT, 0777,0);
    }
}