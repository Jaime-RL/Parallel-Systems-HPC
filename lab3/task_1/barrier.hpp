#pragma once

#define OMPI_SKIP_MPICXX 1 // needed to avoid header issues with C++-supporting MPI implementations 

#include <mpi.h>

void barrier(MPI_Comm);
