#include <stdio.h>
#include <mpi.h>
#include <vector>
#include <windows.h>
#include "MonteCarloSimulation.h"
#include "Sequential.h"

using namespace std;
// Stephen
// USE MPI_Wtime() TO TIME THE EXECUTION OF THE PARALLEL REGION
// Remember to calculate Ts, Tp, Speedup, and Efficiency

void MPIParallel(long double latestAnnualRevenue, double annualGrowthRate, double annualVolatilityRate) {

	// Time variables for sequential and MPI execution times
	double sequentialTime = 0.0;
	double MPITime = 0.0;

	// CONFIGURE MPI ENVIRONMENT
	int rank, size;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Get the number of logical CPUs available on the system
	int logicalCPU = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
	
	// Display the number of logical CPUs and MPI processes only from the root process (rank 0)
	if (rank == 0) {
		printf("Total Logical CPUs: %d\n", logicalCPU);
		printf("Total MPI Processes: %d\n", size);
	}

	// TODO: Need to make so it forecast for Year 2027 2028 and 2029 as well. Currently it only forecasts for 2026
	// Code implementation
	
	// Sequential Operation
	double runtime = MPI_Wtime();
	SequentialOperation(latestAnnualRevenue, annualGrowthRate, annualVolatilityRate);
	sequentialTime = MPI_Wtime() - runtime;

	// MPI Parallel Operation
	runtime = MPI_Wtime();
	
	// TODO: Implement the MPI parallel operation here.

	MPITime = MPI_Wtime() - runtime;
}