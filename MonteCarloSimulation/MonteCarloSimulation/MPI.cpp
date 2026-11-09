#include <stdio.h>

// Stephen
// USE MPI_Wtime() TO TIME THE EXECUTION OF THE PARALLEL REGION

void MPIOperation(long double yearMinusOne, long double yearMinusTwo, long double yearMinusThree, double volatilityRate) {
	printf("\nMPI test\n");

	// Debugging usage
	printf("Year Minus One Accepted: %.2Lf\n", yearMinusOne);
	printf("Year Minus Two Accepted: %.2Lf\n", yearMinusTwo);
	printf("Year Minus Three Accepted: %.2Lf\n", yearMinusThree);
	printf("Volatility Rate Accepted: %.2lf%%\n\n", volatilityRate);
}