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
vector<double> global(SIZE); //For storing computed prefix sums
vector<string> msgArr; 
int num_threads;

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
    msgArr.push_back(myMsg);
}

int main(int argc, char **argv) {
    //Initializing all elements of array by i+1
    for (int i = 0; i < global.size(); i++) global.at(i+1);

    //Obtaining total # of threads through command line
    num_threads = atoi(argv[1]);
}