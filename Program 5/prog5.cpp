//#include <mpi.h>      //For MPI functions, etc 
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
using namespace std;

//Function prototypes
int Compare();
void merge();

//Global variables
int n, arrSize, thread_count, p;

int main(int argc, char **argv) {
	//For testing purposes
	cout << "Hello world\n";
	
	//Reading in n (total number of elements in the array)
	n = stoi(argv[1]), thread_count = stoi(argv[2]);
	
	//Getting problem size (pSize)
	p = n/thread_count;

	//Seeding random numbers 	
	srand(time(NULL));
	
//	MPI_Init(NULL,NULL); //Starting MPI session

	//Creating a dynamic array of ints of size p
	int *arr = new int[p];

	//Filling random numbers in the array
	for (int i = 0; i < n; i++) arr[i] = rand() % 100;

	//Displaying unsorted array
	cout << "Original array: \n";
    	for (int i = 0; i < n; i++) {
		if (i % p == 0 && i < 0) cout << endl;
		cout << arr[i] << " ";  
    	}
    	cout << endl;

	
}
