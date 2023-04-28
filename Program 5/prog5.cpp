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
void merge(int localArr[], int partnerArr[], int local_n, int localPartner[]) { 
	int temp = 2*local_n;
	int local = 0; //For local array index
	int partner = 0; //For partner array index
	int local_partner = 0; //For local + partner array index
	
	while (local_partner < temp) {
		if (local >= temp) {
			while (partner < temp) { 
				localPartner[local_partner] = partnerArr[partner];
				local_partner++;
				partner++;
			}
		}
		if (partner >= temp) {
			while (local < temp) {
				localPartner[local_partner] = localArr[local];
				local_partner++;
				local++;
			}
		}
		
		if (localArr[local] <= partnerArr[partner]) {
			localPartner[local_partner] = localArr[local];
			local_partner++;
			local++;
		}
		
		else {
			localPartner[local_partner] = partnerArr[partner];
			local_partner++;
			partner++;
		}
	}
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
	
	//For time checking
	double start, finish;

	MPI_Init(NULL,NULL); //Starting MPI session
	start = MPI_Wtime(); //Starting stopwatch for MPI execution
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); //Getting number of processesd from -n -x
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //Getting rank
	
	if (my_rank == 0) { 
		n = stoi(argv[1]); //Getting command line argument for n if P0
		MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);
	}

	//Getting problem size (pSize)
	local_n = n/comm_sz;

	//Creating a dynamic array/local list of ints of size n
	int *localArr = new int[n];

	//Filling random numers in the array
	//Seeding random numbers 	
	srand(time(NULL));
	for (int i = 0; i < local_n; i++) localArr[i] = rand() % 100;

	//----------------FIXME--------------
	//Displaying unsorted array in each process
	cout << "Process_ " << my_rank << ", local list:\n";
    	for (int i = 0; i < local_n; i++) {
		//if (i % local_n == 0 && i < 0) cout << endl;
		cout << localArr[i] << " ";  
    	}
    	cout << endl;

	//Calling GNU library quicksort
	qsort(localArr,local_n,sizeof(int),compare);

	//Display sorted array in each process
	cout << "Process_ " << my_rank << ", sorted list:\n";
	for (int i = 0; i < local_n; i++) cout << localArr[i] << " ";

	//----------------FIXME--------------
	//Tree reduction (merge) part
	int divisor = 2;
	int core_diff = 1;
	while (divisor <= comm_sz) { 
		if (my_rank % divisor == 0) { //Receiver
			int *templist = new int[local_n]; //Dynamic array
			int *localArr2 = new int[local_n]; //Used for copying old local array to new local array
			int partner = my_rank + core_diff; //Determine partner
			MPI_Recv(&templist,n,MPI_INT,partner,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Receive local list from partner using MPI_Recv
			merge(localArr,templist,local_n,localArr2); //Call merge function here
			local_n *= 2;
			delete[] templist;
			delete[] localArr;
			localArr = localArr2;
		}

		else { //Sender
			int x = my_rank - core_diff;
			MPI_Send(localArr,local_n,MPI_INT,x,0,MPI_COMM_WORLD);
			delete[] localArr;
			break;
		}
		divisor *= 2;
		core_diff *= 2;
	}

	finish = MPI_Wtime(); //Finishing stopwatch for MPI execution
	
	//----------------FIXME--------------
	//Display entire sorted array after merge operation
	if (my_rank == 0) {
		cout << "Sorted array from process_" << my_rank << "\n";
		for (int i = 0; i < local_n; i++) cout << localArr[i] << " "; 
    		cout << endl;
	}
	delete[] localArr;

	//Display execution time
	cout << "Parallel exec time (sec): " << finish-start << endl;

	//Terminating MPI session
	MPI_Finalize();
	
	return 0;
}
