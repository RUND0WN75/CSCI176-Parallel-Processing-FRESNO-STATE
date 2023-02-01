#include <iostream>
#include <unistd.h> //fork, pipe, wait
#include <vector>
using namespace std;

//Recursive fibonacci
unsigned long long rec(int n) {
	if (n == 0 || n == 1) return n;
	else return rec(n - 1) + rec(n - 2);
}

//Iterative fibonacci
unsigned long long iter(int n) {
	unsigned long long prev = 0; cur = 1, next = prev + cur;
	if (n == 0 || n == 1) return n;
	for (int i = 2; i < n; i++) {
		prev = cur;
		cur = next;
		next = prev + cur;
	}
	return next;
}

vector<int> proc(2); //Process A and B

int main() {
	int status;
	status = pipe(proc);
	int pid1 = fork();

	if (pid1 == 0) {
		int pid2 = fork();
		if (pid2 == 0) iter(1);
	}

	else rec(1);

	wait(&status);
	wait(&status);

	cout << "Hello world\n"; //LATEST CHANGE FOR TESTING PURPOSES
}