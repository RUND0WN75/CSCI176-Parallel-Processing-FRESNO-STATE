/*
Assignment abstract: an assignment demonstrating task-parallel programming through a Unix-pipe utilizing the message-passing communication mechanism. 
This parallel programming method is tested using 3 fibonacci numbers. 
The goal of this program is to compare parallel execution times of the recursive and iterative versions of the fibonacci series using the SPMD task-parallel model. 
The program can be executed with the following command: ./a.exe N --where N is some number.
*/
#include <iostream>
#include <unistd.h> //fork, pipe
#include <sys/wait.h> //wait()
#include <cstdlib> //atoi(), exit(0)
#include <chrono>
#include <iomanip>
using namespace std;

//Recursive fibonacci
unsigned long long rec(int n) {
	if (n == 0 || n == 1) return n;
	else return rec(n - 1) + rec(n - 2);
}

//Iterative fibonacci
unsigned long long iter(int n) {
	unsigned long long prev = 0, cur = 1, next = prev + cur;
	if (n == 0 || n == 1) return n;
	for (int i = 2; i < n; i++) {
		prev = cur;
		cur = next;
		next = prev + cur;
	}
	return next;
}

int main(int argc, char** argv) {
	int pipe1[2];
	int pipe2[2];
	int pipe3[2];
	int pipe4[2];
	pipe(pipe1);
	pipe(pipe2);
	pipe(pipe3);
	pipe(pipe4);

	int n = atoi(argv[1]);
	int pid;
	int status;

	for (int i = 0; i < 3; i++) {

		pid = fork();
		float exec_time1 = 0.0;
		float exec_time2 = 0.0;

		if (pid == 0) {
			//Child 1
			if (i == 0) {
				close(pipe1[0]);
				close(pipe2[1]);

				//Sending n to child 2 & 3
				write(pipe1[1], &n, sizeof(int));
				write(pipe2[1], &n, sizeof(int));

				//Reading exec times from child 2 & 3
				read(pipe3[0], &exec_time1, sizeof(float));
				read(pipe4[0], &exec_time2, sizeof(float));

				cout << "Exec time for recursive version: " << setprecision(5) << exec_time1 << endl;
				cout << "Exec time for iterative version: " << setprecision(5) << exec_time2 << endl;

				close(pipe1[1]);
				close(pipe2[0]);

				break;
			}

			//Child 2
			else if (i == 1) {
				
				close(pipe1[1]);
				close(pipe2[0]);
				
				//Read n from child1
				read(pipe1[0], &n, sizeof(int));

				auto time1 = chrono::high_resolution_clock::now();
				rec(n);
				auto time2 = chrono::high_resolution_clock::now();
				chrono::duration<float, milli> duration1 = time2 - time1;
				//cout << duration1.count() << endl;

				exec_time1 = duration1.count();

				//Sending exec time to child1
				write(pipe3[1], &exec_time1, sizeof(float));

				break;
			}

			//Child 3
			else if (i == 2) {
				close(pipe1[1]);
				close(pipe2[0]);

				//Read n from child1
				read(pipe2[0], &n, sizeof(int));

				auto time3 = chrono::high_resolution_clock::now();
				iter(n);
				auto time4 = chrono::high_resolution_clock::now();
				chrono::duration<float, milli> duration2 = time4 - time3;
				//cout << duration2.count() << endl;

				exec_time2 = duration2.count();

				//Sending exec time to child1
				write(pipe4[1], &exec_time2, sizeof(float));

				close(pipe2[1]);
				break;
			}
		}
	}
	for(int i = 0; i < 3; i++) { 
		pid = wait(nullptr);	
		//cout << "Child " << pid << " exited, status: " << status << endl;
	}
}
