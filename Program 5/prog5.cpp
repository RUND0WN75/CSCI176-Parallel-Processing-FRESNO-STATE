/*
Assignment abstract: an assignment utilizing MPI performing merge sort. The program will start by dividing parts of a local array to some number of processes. 
The qsort() function will then be called from the GNU library to perform quicksort for each process. The merge operation is performed using the tree reduction method, which consists of merging the sorted array
parts into one global array, while still maintaining its order after sorting. The program will output the result of all of the merged local arrays that were sorted by each process, as well as the execution time of the entire operation. 
Program execution was performed on Ubuntu Linux OS 22.04 LTS. 
 
Program can be run with the following command- for 2 and 4 processes: mpiexec -n p ./a.out 400 (p = 2 or 4), for 8 processes: mpiexec --oversubscribe -n 8 ./a.out 400 
(--oversubscribe was needed due to my OS environment's inability to handle more slots)
*/

#include <mpi.h>      //For MPI functions, etc 
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
using namespace std;

//Function prototypes
int compare(const void *a_p, const void *b_p); //Needed for qsort
void merge (int localArr[], int partnerArr[], int local_n, int localPartner[]); //Merge function

//A function to compare two array elements
int compare(const void *a_p, const void *b_p) {
    int a = *((int*)a_p), b = *((int*)b_p);
    if (a < b) return -1;
    else if (a == b) return 0;
    else return 1; //(a > b)
}

//Merge function
void merge(int localArr[], int partnerArr[], int localPartner[], int local_n) { 
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
	
	if (my_rank == 0) n = stoi(argv[1]); //Getting command line argument for n if P0
	MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);

	//Getting problem size (pSize)
	local_n = n/comm_sz;

	//Creating a dynamic array/local list of ints of size n
	int *localArr = new int[local_n];
	
	//Needed to address race conditions, threads with a rank higher than 0 need to wait
	int *temp = new int[1];
	temp[0] = 1;
	if (my_rank > 0) MPI_Recv(new int[1],1,MPI_INT,my_rank - 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

	//Seeding random numbers 	
	srand(time(0)+my_rank);

	//Filling random numers in the array
	for (int i = 0; i < local_n; i++) localArr[i] = rand() % 100;

	//Displaying unsorted array in each process
	cout << "Process_ " << my_rank << ", local list:\n";
    	for (int i = 0; i < local_n; i++) cout << localArr[i] << " ";
    	cout << endl;
    	
    	//Address more race conditions here
	if (my_rank + 1 < comm_sz) MPI_Send(temp,1,MPI_INT,my_rank + 1,0,MPI_COMM_WORLD);
	else MPI_Send(temp,1,MPI_INT,0,0,MPI_COMM_WORLD);

	//Calling GNU library quicksort
	qsort(localArr,local_n,sizeof(int),compare);

	//Address more race conditions here
	if (my_rank > 0) MPI_Recv(new int[1],1,MPI_INT,my_rank - 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	else MPI_Recv(new int[1],1,MPI_INT,comm_sz - 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	
	//Display sorted array in each process
	cout << "Process_ " << my_rank << ", sorted list:\n";
	for (int i = 0; i < local_n; i++) cout << localArr[i] << " ";
	cout << endl;

	//Address more race conditions here	
	if (my_rank + 1 < comm_sz) MPI_Send(temp,1,MPI_INT,my_rank + 1,0,MPI_COMM_WORLD);
	else MPI_Send(temp,1,MPI_INT,0,0,MPI_COMM_WORLD);
	
	if (my_rank == 0) MPI_Recv(new int[1],1,MPI_INT,comm_sz - 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

	//Tree reduction (merge) part
	int divisor = 2;
	int core_diff = 1;
	while (divisor <= comm_sz) { 
		if (my_rank % divisor == 0) { //Receiver
			int *templist = new int[local_n]; //Dynamic array
			int *localArr2 = new int[local_n*2]; //Used for copying old local array to new local array
			int partner = my_rank + core_diff; //Determine partner
			MPI_Recv(templist,local_n,MPI_INT,partner,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Receive local list from partner using MPI_Recv
			merge(localArr,templist,localArr2,local_n); //Call merge function here
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
	
	//Display entire sorted array after merge operation
	if (my_rank == 0) {
		cout << "Sorted array from process_" << my_rank << ":\n";
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
