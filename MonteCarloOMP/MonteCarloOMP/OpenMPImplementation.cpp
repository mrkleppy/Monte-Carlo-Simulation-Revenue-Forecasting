#include <stdio.h>
#include <omp.h>
#include <random>

#include "MonteCarloSimulation.h"
#include "Sequential.h"

void OpenMPParallel(long double latestAnnualRevenue, double annualGrowthRate, double annualVolatilityRate, int firstForecastYear) {
	const int steps = static_cast<int>(NUM_OF_STEPS);

	// Sequential baseline(Ts)
	double start = omp_get_wtime();
	ForecastResult sequentialResult = SequentialOperation(latestAnnualRevenue, annualGrowthRate, annualVolatilityRate);
	double Ts = omp_get_wtime() - start;

	// OpenMP version(Tp)
	ForecastResult parallelResult = {};
	long double totals[FORECAST_YEARS] = {};
	int threadsUsed = 1;
	const unsigned int baseSeed = std::random_device{}();

	start = omp_get_wtime();

#pragma omp parallel num_threads(16)
	{
		// Each thread gets its own random generator 
		std::mt19937 gen(baseSeed + omp_get_thread_num());
		std::uniform_int_distribution<int> shock(-2, 2);

		// Each thread accumulates privately
		long double mySums[FORECAST_YEARS] = {};

#pragma omp single 
		threadsUsed = omp_get_num_threads();

#pragma omp for
		for (int sim = 0; sim < steps; ++sim) {
			long double revenue = latestAnnualRevenue;

			for (int y = 0; y < FORECAST_YEARS; ++y) {
				revenue *= (1 + annualGrowthRate + annualVolatilityRate * shock(gen));
				mySums[y] += revenue;
			}
		}

		// Merge each thread's sums once at the end 
#pragma omp critical
		{
			for (int y = 0; y < FORECAST_YEARS; ++y) {
				totals[y] += mySums[y];
			}
		}
	}

	// Average of all simulations 
	for (int y = 0; y < FORECAST_YEARS; ++y) {
		parallelResult.meanRevenue[y] = totals[y] / static_cast<long double>(steps);
	}

	double Tp = omp_get_wtime() - start;

	// Output 
	printf("\nForecasted Revenues\n");
	printf("Assumption: constant growth and uncertainty parameters.\n");

	printf("\n%-12s", "Method");
	for (int y = 0; y < FORECAST_YEARS; ++y) {
		printf(" | Year %-14d", firstForecastYear + y);
	}
	
	printf("\n");
	for (int i = 0; i < 105; ++i) {
		printf("-");
	}

	printf("\n%-12s", "Sequential");
	for (int y = 0; y < FORECAST_YEARS; ++y) {
		printf(" | RM %s%16s%s", BLUE, formatWithCommas(sequentialResult.meanRevenue[y]).c_str(), RESET);
	}

	printf("\n%-12s", "OpenMP");
	for (int y = 0; y < FORECAST_YEARS; ++y) {
		printf(" | RM %s%16s%s", BLUE, formatWithCommas(parallelResult.meanRevenue[y]).c_str(), RESET);
	}

	double speedup = Ts / Tp;
	double efficiency = speedup / threadsUsed;

	printf("\n\nPerformance Comparison\n");
	printf("-----------------------------------\n");
	printf("Simulations: %d\n", steps);
	printf("OpenMP threads: %d\n", threadsUsed);
	printf("\nTs: %s%.6f seconds%s\n", ORANGE, Ts, RESET);
	printf("Tp: %s%.6f seconds%s\n", ORANGE, Tp, RESET);
	printf("Speedup: %s%.4f%s\n", LIME, speedup, RESET);
	printf("Efficiency: %s%.2f%%%s\n", LIME, efficiency * 100.0, RESET);

	printf("Press Enter to Continue...\n");
	int ch;
	while ((ch = getchar()) != '\n' && ch != EOF) {}
	getchar();
	flush();
}