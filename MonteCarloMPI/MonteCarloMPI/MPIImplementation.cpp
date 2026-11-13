#include <stdio.h>
#include <mpi.h>

#include "MonteCarloSimulation.h"
#include "Sequential.h"

using namespace std;

// Stephen

void MPIParallel(long double latestAnnualRevenue, double annualGrowthRate, double annualVolatilityRate, int firstForecastYear) {

	double runtime = 0.0;
	// Time variables for sequential and MPI execution times
	double Ts = 0.0;
	double Tp = 0.0;

	// CONFIGURE MPI ENVIRONMENT
	int rank = 0;
	int size = 0;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	const long long totalSimulations = static_cast<long long>(NUM_OF_STEPS);

	// Store each year's forecasted value 
	ForecastResult sequentialResult = {};
	ForecastResult parallelResult = {};

	//////////////////////////
	// Sequential Operation //
	// ///////////////////////
	// Only Rank 0 runs the baseline
	if (rank == 0) {
		runtime = MPI_Wtime();
		sequentialResult = SequentialOperation(latestAnnualRevenue, annualGrowthRate, annualVolatilityRate);
		Ts = MPI_Wtime() - runtime;
	}

	
	////////////////////////
	// Parallel Operation //
	// /////////////////////
	// MPI Parallel Operation
	MPI_Barrier(MPI_COMM_WORLD); // Other processes wait until sequential baseline finishes.

	runtime = MPI_Wtime();

	// Divide the scenarios as evenly as possible
	const long long baseCount = totalSimulations / size;
	const long long remainder = totalSimulations % size;

	const long long localCount = baseCount + (rank < remainder ? 1 : 0);

	long double localSums[FORECAST_YEARS] = {};
	long double globalSums[FORECAST_YEARS] = {};

	for (long long simulation = 0; simulation < localCount; ++simulation) {
		// Start each scenario from the latest actual revenue.
		long double revenue = latestAnnualRevenue;

		for (int year = 0; year < FORECAST_YEARS; ++year) {
			revenue = oneSimulation(revenue, annualGrowthRate, annualVolatilityRate);

			localSums[year] += revenue;
		}
	}

	// Combine every forecast year's sum from all processes.
	MPI_Reduce(
		localSums,
		globalSums,
		FORECAST_YEARS,
		MPI_LONG_DOUBLE,
		MPI_SUM,
		0,
		MPI_COMM_WORLD
	);

	// Rank 0 uses the Monte Carlo to find the revenue forecasted for each years
	if (rank == 0) {
		for (int year = 0; year < FORECAST_YEARS; ++year) {
			parallelResult.meanRevenue[year] = (1 / static_cast<long double>(NUM_OF_STEPS)) * globalSums[year];
		}
	}

	double localElapsed = MPI_Wtime() - runtime;

	// Select the largest measured process duration. Put it in Tp
	MPI_Reduce(
		&localElapsed,
		&Tp,
		1,
		MPI_DOUBLE,
		MPI_MAX,
		0,
		MPI_COMM_WORLD
	);

	// Print only after the timed computations.
	if (rank == 0) {
		printf("\nForecasted Revenues\n");
		printf("Assumption: constant growth and uncertainty parameters.\n");
		printf("\n%-12s", "Method");

		for (int year = 0; year < FORECAST_YEARS; ++year) {
			printf(" | Year %-14d", firstForecastYear + year);
		}

		printf("\n");

		for (int i = 0; i < 105; ++i) {
			printf("-");
		}

		printf("\n%-12s", "Sequential");

		for (int year = 0; year < FORECAST_YEARS; ++year) {
			printf(" | %sRM %16s%s", BLUE, formatWithCommas(sequentialResult.meanRevenue[year]).c_str(), RESET);
		}

		printf("\n%-12s", "MPI");

		for (int year = 0; year < FORECAST_YEARS; ++year) {
			printf(" | %sRM %16s%s", BLUE, formatWithCommas(parallelResult.meanRevenue[year]).c_str(), RESET);
		}

		printf("\n\nPerformance Comparison\n");
		printf("----------------------------------------\n");
		printf("Scenarios: %lld\n", totalSimulations);
		printf("Forecast years per scenario: %d\n", FORECAST_YEARS);
		printf("MPI processes: %d\n", size);

		printf("\nSequential Time Taken: %s%.6f seconds%s\n", ORANGE, Ts, RESET);
		printf("Parallel Time Taken: %s%.6f seconds%s\n", ORANGE, Tp, RESET);

		double speedup = Ts / Tp;
		double efficiency =
			speedup / static_cast<double>(size);

		printf("Speedup: %s%.4f%s\n", LIME, speedup, RESET);
		printf("Efficiency: %s%.2f%%%s\n", LIME, efficiency * 100.0, RESET);
	}
}