/*
Name:

Class: CSCI 176 Parallel Processing

Assignment abstract: an assignment utilizing the OpenMP API.

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

int compare(const void *a_p, const void *b_p) {
    int a = *((int*)a_p), b = *((int*)b_p);
    if (a < b) return -1;
    else if (a == b) return 0;
    else return 1; //(a > b)
}

int check_sorted(int *arr, int n) {
    for (int i = 0; i < n-1; i++) if (arr[i] > arr[i+1]) return 0; //Not sorted
    return 1; //Sorted
}

//localN = # of elements / # of threads
void merge_part(int *arr,int n, int start1, int start2) {
    int divisor = 2, core_diff = 1;
    int rank = omp_get_thread_num(), thread_count = omp_get_num_threads();
    int localN = n/thread_count, i = 0; //localN = problem size
    while (divisor <= thread_count) {
        //Barrier
        #pragma omp barrier

        if (rank % divisor == 0) { //Receiver
            start1 = rank * localN;
            start2 = (rank + core_diff) * localN;
            merge_part(arr,n,start1,start2);
        }

        divisor *= 2;
        core_diff *= 2;
    }

    cout << "Thread ID: " << rank << ", local_list: " << arr[i] << endl;
}

int main(int argc, char **argv) {
    //n = number of list elements, num_threads = number of threads
    int n = stoi(argv[2]), thread_count = stoi(argv[1]);
    
    //Getting problem size
    int pSize = n / thread_count, core_diff = 1;

    //Creating global and local arrays
    int *globalArr = new int[n], *localArr = new int[pSize];

    int start = thread_count * pSize, end = (thread_count + core_diff) * pSize;

    for (; thread_count >= 1; thread_count /= 2) {} //To be used for aalmost all of main?

    //Time checking
    omp_get_wtime();
    #pragma omp parallel for num_threads(thread_count)
    
    //Filling array with random numbers
    for (int i = 0; i < n; i++) globalArr[i] = rand() % n + 1;
    
    //Displaying original array
    cout << "Original array: ";
    for (int i = 0; i < n; i++) cout << globalArr[i] << " "; 
    cout << endl;

    //GNU library quicksort
    qsort(localArr,n,sizeof(int),compare);

    //Copying elements from global array to local array
    #pragma omp parallel for
    for (int i = 0; i < n; i++) localArr[i] = globalArr[i+start];

    //Implementing merge portion
    for (int i = 0; i < thread_count; i++) merge_part(globalArr, n, 0,0);

    //Displaying sorted array
    cout << "Sorted array: ";
    for (int i = 0; i < n; i++) cout << globalArr[i] << " ";
    cout << endl;

    //Checking to verify that the array is sorted
    if (check_sorted(globalArr,n) == 1) cout << "Array is sorted\n";
    else cout << "Array is not completely sorted\n";
}