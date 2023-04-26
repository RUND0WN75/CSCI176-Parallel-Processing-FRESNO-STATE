#include <mpi.h>      //For MPI functions, etc 
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
using namespace std;

int compare(const void *a_p, const void *b_p); //Needed for qsort

//A function to compare two array elements
int compare(const void *a_p, const void *b_p) {
    int a = *((int*)a_p), b = *((int*)b_p);
    if (a < b) return -1;
    else if (a == b) return 0;
    else return 1; //(a > b)
}

//----------------FIXME--------------
//Merge function
void merge(int arr1[], int arr2[], int local_n) { 
	int *temp = new int[2*local_n]; //Temporary list
	//Copy elements from temporary list to arr1
	delete[] temp;
}


int main(int argc, char **argv) {
	//For testing purposes
	cout << "Hello world\n";

	/*
	comm_sz = command size
	local_n = problem size
	my_rank = thread rank
	n = number of elements in the array 
	*/
	int n, comm_sz, local_n, my_rank;

	MPI_Init(NULL,NULL); //Starting MPI session
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); //Getting number of processesd from -n -x
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //Getting rank
	
	if (my_rank == 0) n = stoi(argv[1]); //Getting command line argument for n if P0
	MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);

	//Getting problem size (pSize)
	local_n = n/comm_sz;

	//Creating a dynamic array/local list of ints of size n
	int *localArr = new int[n];

	//Filling random numers in the array
	//Seeding random numbers 	
	srand(time(NULL));
	for (int i = 0; i < n; i++) localArr[i] = rand() % 100;

	//----------------FIXME--------------
	//Displaying unsorted array in each process
	cout << "Original array: \n";
    	for (int i = 0; i < n; i++) {
		if (i % local_n == 0 && i < 0) cout << endl;
		cout << localArr[i] << " ";  
    	}
    	cout << endl;

	//Calling GNU library quicksort
	qsort(localArr,local_n,sizeof(int),compare);

	//----------------FIXME--------------
	//Display sorted array in each process

	//----------------FIXME--------------
	//Tree reduction (merge) part
	int divisor = 2;
	int core_diff = 1;
	while (divisor <= comm_sz) { 
		if (my_rank % divisor == 0) { //Receiver
			int *templist = new int[n]; //Dynamic array 
			//Determine partner
			MPI_Recv(&templist,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Receive local list from partner using MPI_Recv
			merge(localArr,templist,local_n); //Call merge function here
			local_n *= 2;
			delete[] templist;
		}

		else { //Sender
			//Determine partner
			MPI_Send(localArr,1,MPI_INT,0,0,MPI_COMM_WORLD);
			//done = 1; sending process terminates while loop
		}
		divisor *= 2;
		core_diff *= 2;
	}

	//----------------FIXME--------------
	//Display entire sorted array after merge operation
	int *globalArr;
	if (my_rank == 0) {
		//DIsplay sorted array	
	}
	delete[] localArr;

	//----------------FIXME--------------
	//Display execution time
	cout << "Parallel exec time (sec): " << endl;

	//Terminating MPI session
	MPI_Finalize();
}
