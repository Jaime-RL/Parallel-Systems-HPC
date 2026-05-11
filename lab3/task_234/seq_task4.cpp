#include "common.hpp"
#include <vector>  

/* Here is the list of functions describing the behaviour of each module, except in() and out() defined in common.cpp */


void S(const int src, const int min_dst, int max_dst) {
    streamelement x;

    while(true) {
        x.recv(src);

        for (int i = min_dst; i <= max_dst; ++i){
            x.send(i);
        }

        if(x.is_terminated()){
            return;
        }    
    }
}

// Function describing the behavior of the module M
void M(const int src, const int dst, int start, int end) {
	streamelement x;
	while(true) {
		x.recv(src);
		if(x.is_terminated()) {
			x.send(dst);
			return;
		}
		for(int j=start; j<=end; ++j)
			x[j] = j1(h1(g1(f1(x[j]))));
		x.send(dst);
	}
}

void G(int dst, int min_src, int max_src, int n_big_partitions, int elem_size) {
    streamelement x[STREAM_LENGTH]; // An array containing all the elements/packages
    streamelement aux; // To receive (partitioned) packages from workers
    int n_workers = max_src - min_src + 1;
    int n_finished_workers = 0;

    // For each worker, stores how much elements the worker has sent
    std::vector<int> worker_elems_sent(n_workers, 0);
    // For each element, stores how many parts have been received
    std::vector<int> parts_received(STREAM_LENGTH, 0);

    while(true){
        int src = aux.recv_any(); //Receives a package from some worker, src is the worker rank

        if(aux.is_terminated()){
            n_finished_workers++;
            if(n_finished_workers == n_workers){
                aux.send(dst);
                return;
            }
            continue; // Skip processing data if it is a termination message
        }

        int worker = src - min_src;
        // If this worker has already sent n elements, this one will be in position n
        int elem_idx = worker_elems_sent[worker];
        
        // The element that we should update with the worker computed part
        streamelement &elem = x[elem_idx];
        int start, end;

        // Compute start an end of the worker's processed part of the element
        if (worker < n_big_partitions){
            start = worker * (elem_size+1);
            end = start + elem_size;
        }
        else{
            start = n_big_partitions*(elem_size+1) + (worker - n_big_partitions)*elem_size;
            end = start + elem_size - 1;
        }

        // Update the final element
        for (int i = start; i <= end; ++i){
            elem[i] = aux[i];
        }

        worker_elems_sent[worker]++;
        parts_received[elem_idx]++;

        if(parts_received[elem_idx] == n_workers){
            elem.send(dst);
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
    int n_workers = required_comm_size - 4;
    int n_elems = STREAM_ELEMENT_SIZE / n_workers;
    int n_big_partitions = STREAM_ELEMENT_SIZE % n_workers;

	if(rank==0){  // IN
        in(1);
    }
	else if(rank==1){ // SPLITTER
        S(0, 2, required_comm_size-3);
    }
    else if(rank < 2 + n_big_partitions){ //WORKERS FOR BIG PARTITIONS
        int worker_n = rank-2;
        int start = worker_n * (n_elems+1);
        int end = start + n_elems;
        M(1, required_comm_size-2, start, end);
    }
    else if (rank < required_comm_size - 2){ //WORKERS FOR REGULAR PARTITIONS
        int worker_n = rank-2;
        int start = n_big_partitions*(n_elems+1) + (worker_n-n_big_partitions)*n_elems;
        int end = start + n_elems - 1;
        M(1, required_comm_size-2, start, end);
    }
	else if(rank==required_comm_size-2){ // GATHERER
        G(required_comm_size-1, 2, required_comm_size-3, n_big_partitions, n_elems);
    }
	else if(rank==required_comm_size-1){ // OUT
        out(required_comm_size-2);
    }
}
