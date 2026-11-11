#include <stdio.h>
#include "Sequential.h"

// Jarrett
// USE omp_get_wtime() TO TIME THE EXECUTION OF THE PARALLEL REGION

void OpenMPParallel(long double latestAnnualRevenue, double annualGrowthRate, double annualVolatilityRate) {
	
	printf("OpenMP test\n");

	// Debugging usage
	printf("Latest Annual Revenue Accepted: %.2Lf\n", latestAnnualRevenue);
	printf("Annual Growth Rate Accepted: %.2f\n", annualGrowthRate);
	printf("Annual Volatility Rate Accepted: %.2f\n", annualVolatilityRate);
}