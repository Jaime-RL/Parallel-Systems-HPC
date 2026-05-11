#include "common.hpp"   

/* Here is the list of functions describing the behaviour of each module, except in() and out() defined in common.cpp */

void E(const int src, const int min_dst, int max_dst) {
    streamelement x;
    int current_dst = min_dst;

    while(true) {
        x.recv(src);
        if(x.is_terminated()) {
            for(int i = min_dst; i <= max_dst; ++i) {
                x.send(i);
            }
            return;
        }
        
        x.send(current_dst);
        current_dst++;
        if (current_dst > max_dst){
            current_dst = min_dst;
        }
    }
}

// Function describing the behavior of the module M
void M(const int src, const int dst) {
	streamelement x;
	while(true) {
		x.recv(src);
		if(x.is_terminated()) {
			x.send(dst);
			return;
		}
		for(int j=0; j<STREAM_ELEMENT_SIZE; ++j)
			x[j] = j1(h1(g1(f1(x[j]))));
		x.send(dst);
	}
}

void C(int dst, int num_workers) {
	streamelement x;
    int workers_done = 0;

    while(true) {
        x.recv_any();        
        if(x.is_terminated()) {
            workers_done++;
            if(workers_done == num_workers) {
                x.send(dst); 
                return;
            }
        } else {
            x.send(dst);
        }
    }
}

/*
Two approaches to calculating required_comm_size

=== THEORETICAL, CODE ANALYSIS ===
Processing an element involves applying F = j1∘h1∘g1∘f1 to each of its positions.
f1 waits for 17ms, g1 waits for 9ms, h1 waits for 7ms, and j1 waits for 6ms.
Therefore, F takes 39ms to compute per integer.
Since each stream element has STREAM_ELEMENT_SIZE integers, the time to fully process
one element is 
    Ts = STREAM_ELEMENT_SIZE*39ms = 16*39ms = 624ms
On the other hand, in applies f0, that waits 12ms, to each element, thus taking
STREAM_ELEMENT_SIZE*12ms too fully send on element, that is,
    Ta = STREAM_ELEMENT_SIZE*12ms = 16*12ms = 192ms
Finally, the theoretical minimum number of workers that solves bottleneck is
    ceil(Ts/Ta) = ceil(3.25) = 4
Since there are also 4 other processes (int, E, C, out), the total number of processes
is 4+4= 8

=== EMPIRICAL, REAL MEASUREMENTS ===
After running the original seq.cpp file, which processes the elements sequentially, we
get:
    completion time = 12.78 sec
    avg service time = 0.64 sec
    avg latency = 0.63 sec
Therefore, Ts = 0.64s = 640ms
In order to empirically obtain the arrival time Ta, we modified seq to measure how
much time passes between the first element arrival and the last, dividied by the
number of elements. The result was:
    Ta = 0.184476s ~= 184ms
So the number of workers that solves the bottleneck using real measurements is
    ceil(Ts/Ta) = ceil(3.47) = 4
Thus, the total number of processes is 4+4=8

Since both theoretical and practical calculations give us the same number, we set:
*/
const int required_comm_size = 8;

// This function maps each module to a distinct rank associated to a process (mind that each process is associated with exactly one module)
void map_rank(const int rank) {
	if(rank==0) in(1);
	else if(rank==1) E(0, 2, required_comm_size-3);
	else if(rank < required_comm_size-2) M(1, required_comm_size-2);
	else if(rank==required_comm_size-2) C(required_comm_size-1, required_comm_size-4);
	else if(rank==required_comm_size-1) out(required_comm_size-2);
}
