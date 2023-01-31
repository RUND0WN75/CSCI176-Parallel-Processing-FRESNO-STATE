#include <iostream>
#include <unistd.h> //fork, pipe, wait
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

int main() {
	
	return 0;
}