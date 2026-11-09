#include <stdio.h>

// Liew
// USE cudaEvent_t TO TIME THE EXECUTION OF THE PARALLEL REGION

void CUDAOperation(long double yearMinusOne, long double yearMinusTwo, long double yearMinusThree, double volatilityRate) {
	printf("\nCUDA test\n");

	// Debugging usage
	printf("Year Minus One Accepted: %.2Lf\n", yearMinusOne);
	printf("Year Minus Two Accepted: %.2Lf\n", yearMinusTwo);
	printf("Year Minus Three Accepted: %.2Lf\n", yearMinusThree);
	printf("Volatility Rate Accepted: %.2lf%%\n\n", volatilityRate);
}