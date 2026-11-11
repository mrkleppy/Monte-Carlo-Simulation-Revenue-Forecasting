#include <stdio.h>
#include <vector>
#include "MonteCarloSimulation.h"

using namespace std;

void SequentialOperation(long double latestAnnualRevenue, double annualGrowthRate, double annualVolatilityRate) {
	if (NUM_OF_STEPS <= 0) {
		printf("\nNumber of steps must be greater than 0.\n");
		return;
	}

	vector<long double>results(static_cast<size_t>(NUM_OF_STEPS));

	for (long long i = 0; i < NUM_OF_STEPS; ++i) {
		results[i] = oneSimulation(latestAnnualRevenue, annualGrowthRate, annualVolatilityRate);
	}

	// Finds the mean of the simulated revenue results
	long double meanForecastRevenue = monteCarloSimulation(results.data());

	// Debugging usage
	printf("Sequential test\n");
	printf("Simulated Revenue: %sRM %.2Lf%s\n", BLU, meanForecastRevenue, RESET);
}