#include <stdio.h>
#include "Sequential.h"

// Liew
// USE cudaEvent_t TO TIME THE EXECUTION OF THE PARALLEL REGION

void CUDAParallel(long double latestAnnualRevenue, double annualGrowthRate, double annualVolatilityRate) {
	printf("CUDA test\n");

	// Debugging usage
	printf("Latest Annual Revenue Accepted: %.2Lf\n", latestAnnualRevenue);
	printf("Annual Growth Rate Accepted: %.2f\n", annualGrowthRate);
	printf("Annual Volatility Rate Accepted: %.2f\n", annualVolatilityRate);
}