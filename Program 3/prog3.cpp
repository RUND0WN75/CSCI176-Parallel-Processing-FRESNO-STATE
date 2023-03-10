/*
Assignment abstract: 
*/
#include <iostream>
#include <vector>
//#include <pthread.h>
//#include <semaphore.h>
#include <cstdlib>
#include <sstream>
using namespace std;
#define SIZE 500000017;

vector<double> global(SIZE);

int main(int argc, char **argv) {
    //Initializing all elements of array by i+1
    for (int i = 0; i < global.size(); i++) global.at(i+1);
}