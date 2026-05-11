#include "common.hpp"
#include <functional>

/* Here is the list of functions describing the behaviour of each module, except in() and out() defined in common.cpp */
void M_f(const int src, const int dst) {
	streamelement x;
	while(true) {
		x.recv(src);
		if(x.is_terminated()) {
			x.send(dst);
			return;
		}
		for(int j=0; j<STREAM_ELEMENT_SIZE; ++j)
			x[j] = f1(x[j]);
		x.send(dst);
	}
}

void M_hg(const int src, const int dst) {
	streamelement x;
	while(true) {
		x.recv(src);
		if(x.is_terminated()) {
			x.send(dst);
			return;
		}
		for(int j=0; j<STREAM_ELEMENT_SIZE; ++j)
			x[j] = h1(g1(x[j]));
		x.send(dst);
	}
}

void M_j(const int src, const int dst) {
	streamelement x;
	while(true) {
		x.recv(src);
		if(x.is_terminated()) {
			x.send(dst);
			return;
		}
		for(int j=0; j<STREAM_ELEMENT_SIZE; ++j)
			x[j] = j1(x[j]);
		x.send(dst);
	}
}

/*
We will approach this by analysing the code

Processing an element involves applying F = j1∘h1∘g1∘f1 to each of its positions.
f1 waits for 17ms, g1 waits for 9ms, h1 waits for 7ms, and j1 waits for 6ms.
Therefore, the biggest bottleneck here is f1. h1∘g1 takes 9+7=16ms to process, which
is smaller than f1, so we can merge h1 and g1.
We can not, however, merge j1 to them since that would take 16+6=22ms, becoming the new
bottleneck.
So we will have three modules: f1, h1∘g1, j1
*/
const int required_comm_size = 5; 

// This function maps each module to a distinct rank associated to a process (mind that each process is associated with exactly one module)
void map_rank(const int rank) {
	if(rank==0) in(1);
	else if(rank==1) M_f(0,2);
	else if(rank==2) M_hg(1,3);
	else if(rank==3) M_j(2,4);
	else if(rank==4) out(3);
}