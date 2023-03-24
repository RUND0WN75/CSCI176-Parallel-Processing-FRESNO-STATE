/*
Name: Craig Walkup

Class: CSCI 176 Parallel Processing

Assignment abstract: 
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

int main(int argc, char **argv) {
    //n = number of list elements, num_threads = number of threads
    int n = stoi(argv[2]), num_threads = stoi(argv[1]);
    int *arr = new int[n];

    #pragma omp parallel for threadCount(num_threads)
    for (int i = 0; i < n; i++) arr[i] = rand() % n + 1;
    
    //Displaying original array
    cout << "Original array: ";
    for (int i = 0; i < n; i++) cout << arr[i] << " "; 

    //GNU library quicksort
    qsort(arr,n,sizeof(int),compare);

    //Displaying sorted array
    cout << "Sorted array\n";
    for (int i = 0; i < n; i++) cout << arr[i] << " ";
    cout << endl;

    //Checking to verify that the array is sorted
    if (check_sorted(arr,n) == 1) cout << "Array is sorted\n";
    else cout << "Array is not completely sorted\n";
}