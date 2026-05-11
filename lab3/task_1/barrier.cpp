#include "barrier.hpp"

void barrier(MPI_Comm COMM_BARRIER) {
	int rank, size;
	int value;

	MPI_Comm_rank(COMM_BARRIER, &rank);
	MPI_Comm_size(COMM_BARRIER, &size);

	if (rank==0){ // first one
		for(int i = 1; i < size; i++){
			MPI_Recv(&value, 1, MPI_INT, i, MPI_ANY_TAG, COMM_BARRIER, MPI_STATUS_IGNORE);
		}

		for(int i = 1; i < size; i++){
			MPI_Ssend(&value, 1, MPI_INT, i, 0, COMM_BARRIER);
		}
	}
	else{
		MPI_Ssend(&value, 1, MPI_INT, 0, 0, COMM_BARRIER);
		MPI_Recv(&value, 1, MPI_INT, 0, MPI_ANY_TAG, COMM_BARRIER, MPI_STATUS_IGNORE);
	}
}
