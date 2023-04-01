/*
Assignment abstract: an assignment utilizing the OpenMP API. The program will consist of two arrays (global and local) with different uses. 

The global array will be filled with random numbers ranging up to a specific number from the command line. 

The local array will be divided up into a specified number of threads to perform quicksort and merge operations.

Each thread will be given a small chunk of the local array to perform those operations. 

4 functions exist: the GNU version quicksort, merge, compare, and check_sorted. These 4 functions help ensure that
the global and local arrays will be properly sorted given the threads that have been assigned to perform the merge operations. 

The program will output the original unsorted global array, followed by each thread with tbeir respective rank and chunk of the local array.
It will then provide the sorted version of the local array for each thread ID, and then the program will merge the thread's work together
into one newly sorted global array. The Program will be timed to determine the execution time of various operations performed on the arrays. 

Program can be run with the following command: ./a.exe p n //p = 1,2,4 or 8, n = 100 or 100000000
*/
#include <iostream>
#include <cstdlib>
#include <thread>
#ifdef _OPENMP
#include <omp.h>
#endif
using namespace std;

//Function prototypes
int compare(const void *a_p, const void *b_p); //Needed for qsort
int check_sorted(int arr[], int n); //Checking if a list is sorted or not

//A function to compare two array elements
int compare(const void *a_p, const void *b_p) {
    int a = *((int*)a_p), b = *((int*)b_p);
    if (a < b) return -1;
    else if (a == b) return 0;
    else return 1; //(a > b)
}

//A function to ensure the array is sorted
int check_sorted(int *arr, int n) {
    for (int i = 0; i < n-1; i++) if (arr[i] > arr[i+1]) return 0; //Not sorted
    return 1; //Sorted
}

//Merge function
void merge(int arr[], int start, int middle) {
    int end = (2*middle) - (start + 1);
    int temp_left = start; //Getting left subarray
    int a = (middle - start); //Size of left subarray
    int *sub_left = new int[a];
    for (int i = 0; i < a; i++) {
        sub_left[i] = arr[i + start];
    }

    int temp_right = middle; //Getting right subarray
    int b = end - middle + 1; //Size of right subarray
    int* sub_right = new int[b];
    for (int j = 0; j < b; j++) {
        sub_right[j] = arr[j + middle];
    }

    temp_left = 0; temp_right = 0;

    for (int k = start; k <= end; k++) {
        if (temp_left >= a) { //When left subarray is empty
            arr[k] = sub_right[temp_right];
            temp_right++;
        }

        else if (temp_right >= b) { //When right subarray is empty
            arr[k] = sub_left[temp_left];
            temp_left++;
        }
        else if (sub_left[temp_left] <= sub_right[temp_right]) {
            arr[k] = sub_left[temp_left];
            temp_left++;
        }

        else if (sub_right[temp_right] < sub_left[temp_left]) {
            arr[k] = sub_right[temp_right];
            temp_right++;
        }
    }
    delete[] sub_left;
    delete[] sub_right;
}

/*
Global variables for the following: 
1. Global array
2. Global array size
3. Thread count
4. pSize (problem size)
*/
int *globalArr;
int globalArrSize, thread_count, pSize;

int main(int argc, char **argv) {
    //n = number of list elements, num_threads = number of threads
    globalArrSize = stoi(argv[2]), thread_count = stoi(argv[1]);
    
    //Needed to seed random numbers
    srand(time(NULL));
    /*
    cout << rand() % globalArrSize + 1 << endl;
    cout << rand() % globalArrSize + 1 << endl;
    cout << rand() % globalArrSize + 1 << endl;
    cout << rand() % globalArrSize + 1 << endl;*/

    //Getting problem size
    pSize = globalArrSize / thread_count; //Work per thread

    //Creating global array
    globalArr = new int[globalArrSize];

    //Filling array with random numbers
    for (int i = 0; i < globalArrSize; i++) globalArr[i] = rand() % globalArrSize + 1;

    //COMMENT OUT THIS SECTION FOR SAMPLE OUTPUT W/O TRACE
    //Displaying original array
    cout << "Original array: \n";
    for (int i = 0; i < globalArrSize; i++) {
        if (i % pSize == 0 && i < 0) cout << endl;
        cout << globalArr[i] << " ";  
    }
    cout << endl;

    //Time checking
    double start_time;
    #pragma omp parallel num_threads(thread_count)
    {
        #pragma omp single
        start_time = omp_get_wtime();

        //Creating local array, getting thread ID and start index
        int *localArr = new int[pSize];
        int rank = omp_get_thread_num();
        int start = rank*pSize;

        //Copying elements from global to local array
        //Outputting rank/thread number with respective local array elements
        #pragma omp critical 
        {
            #pragma omp parallel for
            for (int i = start; i < start + pSize; i++) { 
                localArr[i - start] = globalArr[i];
                //cout << localArr[i - start] << " ";
            }
            cout << "Thread_" << rank << ", local_list: ";
            for (int i = 0; i < pSize; i++) cout << localArr[i] << " ";
            cout << endl;
        }
        //GNU library quicksort
        qsort(localArr,pSize,sizeof(int),compare);
        #pragma omp critical 
        {
            cout << "Thread_" << rank << ", sorted local_list: ";
            //Outputting sorted local array using threads
            for (int i = 0; i < pSize; i++) cout << localArr[i] << " ";
            cout << endl;
        }
        //Copying elements of local array back to global array
        for (int i = start; i < start + pSize; i++) globalArr[i] = localArr[i-start];

        delete[] localArr;

        //Implementing merge portion
        int core_diff = 1;
        int divisor = 2;
        while (divisor <= thread_count) {
            #pragma omp barrier //Barrier
            if (rank % divisor == 0) { //Receiver
                int start2 = (rank + core_diff) * pSize;
                merge(globalArr,start,start2);
            }
            divisor *= 2;
            core_diff *= 2;
        }
    }

    //Time checking
    double end_time = omp_get_wtime();

    //COMMENT OUT THIS SECTION FOR SAMPLE OUTPUT W/O TRACE
    //Displaying sorted array
    cout << "Sorted array: ";
    for (int i = 0; i < globalArrSize; i++) cout << globalArr[i] << " "; 
    cout << endl;

    //Checking to verify that the array isstartsorted
    if (check_sorted(globalArr,globalArrSize)) cout << "Array is sorted\n";
    else cout << "Array is not completely sorted\n";

    //Need to output execution time
    cout << "Using " << thread_count << " threads and " << globalArrSize << " elements, time taken: " << (double)(end_time-start_time) << " seconds." << endl;
}