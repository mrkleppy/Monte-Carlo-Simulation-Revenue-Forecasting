#include <stdio.h>
#include "MonteCarloSimulation.h"

using namespace std;

ForecastResult SequentialOperation(long double latestAnnualRevenue, double annualGrowthRate, double annualVolatilityRate) {

	ForecastResult result = {};

	long double sums[FORECAST_YEARS] = {};

	for (long long simulation = 0; simulation < NUM_OF_STEPS; ++simulation) {
		// Start each scenario from the latest actual revenue
		long double revenue = latestAnnualRevenue;

		for (int year = 0; year < FORECAST_YEARS; ++year) {
			// Carry this scenario's simulated revenue forward
			revenue = oneSimulation(revenue, annualGrowthRate, annualVolatilityRate);

			sums[year] += revenue;
		}
		
	}

	// Monte Carlo Formula to find the revenue that is found
	// Formula is
	// 1/N (Sum of All Simulated Revenue in a year)
	// 1/NUM_OF_STEPS * SUM(revenues)
	for (int year = 0; year < FORECAST_YEARS; ++year) {
		result.meanRevenue[year] = (1 / static_cast<long double>(NUM_OF_STEPS)) * sums[year];
	}
	return result;
}