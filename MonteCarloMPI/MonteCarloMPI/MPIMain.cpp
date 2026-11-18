#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <cmath>
#include <cerrno>
#include <cctype>
#include <mpi.h>
#include <Windows.h>

#include "MonteCarloSimulation.h"
#include "MPIImplementation.h"

using namespace std;

// Convert a complete command-line argument into a positive revenue
bool parseRevenue(const char* text, long double* revenue) {
	char* end = nullptr;

	errno = 0;

	long double value = strtold(text, &end);

	// Reject empty conversion or out-of-range value
	if (end == text || errno == ERANGE) {
		return false;
	}

	// Allow trailing whitespace, but not other trailing characters
	while (*end != '\0' && isspace(static_cast<unsigned char>(*end))) {
		end++;
	}

	if (*end != '\0' || !isfinite(value) || value <= 0.0L) {
		return false;
	}

	*revenue = value;

	return true;
}

int main(int argc, char* argv[]) {
	setvbuf(stdout, NULL, _IONBF, 0);

	// CONFIGURE MPI ENVIRONMENT
	MPI_Init(&argc, &argv);

	int rank = 0;
	int size = 0;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	long double precedingAnnualRevenue = 0.0L;
	long double latestAnnualRevenue = 0.0L;

	double annualGrowthRate = 0.0;
	double annualVolatilityRate = 0.0;

	int inputValid = 1;
	int latestRevenueYear = 0;
	int precedingRevenueYear = 0;

	// Only the root process reads arguments and prepares the model
	if (rank == 0) {
		if (argc != 3) {
			// Prompt error and proper usage of MPI
			fprintf(
				stderr,
				"Usage:\n"
				"MonteCarloMPI.exe <precedingRevenue> <latestRevenue>\n"
				"Example:\n"
				"mpiexec -n 16 ./MonteCarloMPI.exe 50000 60000\n\n"
			);

			inputValid = 0;
		}

		// If the argument input is valid go here (Now check if the 2 arguments given is valid)
		if (inputValid) {
			inputValid = parseRevenue(argv[1], &precedingAnnualRevenue) && parseRevenue(argv[2], &latestAnnualRevenue);

			if (!inputValid) {
				fprintf(
					stderr,
					"Invalid revenue arguments. Enter positive finite numbers without commas.\n");
			}
		}

		// If argument passes the check
		// Now we get current time for display purposes (Year 2024, 2025, 2026, 2027, 2028, 2029) Depends on system time
		// This is to display the latest and preceding year
		if (inputValid) {
			time_t currentTime = time(NULL);
			tm localTime = {};

			if (localtime_s(&localTime, &currentTime) != 0) {
				fprintf(stderr, "Error getting local time.\n");
				inputValid = 0;
			}
			else {
				const int currentYear = localTime.tm_year + 1900;

				latestRevenueYear = currentYear - 1;
				precedingRevenueYear = currentYear - 2;
			}
		}

		// Get the number of logical CPUs available on the system
		int logicalCPU = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);

		// Display the number of logical CPUs and MPI processes only from the root process (rank 0)
		if (rank == 0) {
			printf("Total Logical CPUs: %d\n", logicalCPU);
			printf("Total MPI Processes: %d\n", size);
		}

		// Display
		if (inputValid) {
			printf("\nMonte Carlo Revenue Forecasting — MPI\n");
			printf("----------------------------------------\n");

			// Display the historical inputs before calculating rates.
			printf("Annual Revenue for Year %d: %sRM %s%s\n", precedingRevenueYear, MAGENTA, formatWithCommas(precedingAnnualRevenue).c_str(), RESET);
			printf("Annual Revenue for Year %d: %sRM %s%s\n", latestRevenueYear, MAGENTA, formatWithCommas(latestAnnualRevenue).c_str(), RESET);

			// Identify the growth rate and volatility rate based on the two years of revenue
			annualGrowthRate = findGrowthRate(latestAnnualRevenue, precedingAnnualRevenue); // How much the revenue has grown from the previous year to the current year
			annualVolatilityRate = findVolatilityRate(latestAnnualRevenue, precedingAnnualRevenue); // How much the revenue has fluctuated from the previous year to the current year (the uncertainties)
		
			// Check if there was infinity
			if (!isfinite(annualGrowthRate) || !isfinite(annualVolatilityRate)) {
				fprintf(
					stderr,
					"Unable to calculate finite model parameters.\n"
				);
			}
			else {
				printf("\nAnnual Growth Rate: %s%.4f%%%s\n", LIME, annualGrowthRate * 100.0, RESET);
				printf("Annual Volatility Rate: %s%.4f%%%s\n", LIME, annualVolatilityRate * 100.0, RESET);
				printf("Number of Simulations: %lld\n", static_cast<long long>(NUM_OF_STEPS));
			}
		}
	}

	// Seed the random number generator
	srand((unsigned int)time(NULL));

	int firstForecastYear = 0;

	if (rank == 0) {
		firstForecastYear = latestRevenueYear + 1;
	}

	// All ranks receive the same proceed/exit decision.
	MPI_Bcast(
		&inputValid,
		1,
		MPI_INT,
		0,
		MPI_COMM_WORLD);

	if (!inputValid) {
		MPI_Finalize();
		return EXIT_FAILURE;
	}

	// Send the prepared model inputs from rank 0 to every rank.
	// rank 0 Broadcast to every other ranks about latestAnnualRevenue variable
	MPI_Bcast(
		&latestAnnualRevenue,
		1,
		MPI_LONG_DOUBLE,
		0,
		MPI_COMM_WORLD);

	// rank 0 Broadcast to every other ranks about annualGrowthRate variable
	MPI_Bcast(
		&annualGrowthRate,
		1,
		MPI_DOUBLE,
		0,
		MPI_COMM_WORLD);

	// rank 0 Broadcast to every other ranks about annualVolatilityRate variable
	MPI_Bcast(
		&annualVolatilityRate,
		1,
		MPI_DOUBLE,
		0,
		MPI_COMM_WORLD);

	// rank 0 Broadcast to every other ranks about fireForecastYear variable
	MPI_Bcast(
		&firstForecastYear,
		1,
		MPI_INT,
		0,
		MPI_COMM_WORLD
	);

	// Begin the Calculation
	MPIParallel(latestAnnualRevenue, annualGrowthRate, annualVolatilityRate, firstForecastYear);

	MPI_Finalize();

	return 0;
}