#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <cmath>
#include <math.h>
#include <mpi.h>
#include "MonteCarloSimulation.h"
#include "OpenMPImplementation.h"
#include "CUDAImplementation.h"
#include "MPIImplementation.h"

void mainMenu() {
	// Get the current time for display purposes
	time_t currentTime = time(NULL);
	tm localTime = {};

	if (localtime_s(&localTime, &currentTime) != 0) {
		printf("Error getting local time.\n");
		return;
	}
	
	const int currentYear = localTime.tm_year + 1900;
	const int latestRevenueYear = currentYear - 1;     // Previous year for which revenue is known
	const int precedingRevenueYear = currentYear - 2;  // Year before the previous year for which revenue is known

	while (1) {
		long double latestAnnualRevenue = 0.0L;
		long double precedingAnnualRevenue = 0.0L;
		int choice = 0;

		printf("Monte Carlo Simulation for Four-Year Revenue Forecasting using Historical Growth\n");
		
		printf("\nChoose Parallel Program to compare with Sequential\n");
		printf("1. OpenMP\n2. CUDA\n3. MPI\n0. Exit Program\nChoice: ");

		if (scanf_s("%d", &choice) != 1) {
			printf("\nInvalid input. Please enter an integer.\n");
			return;
		}

		if (choice == 0) {
			printf("\nExiting program...");
			return;
		}

		if (choice < 1 || choice > 3) {
			printf("\nPlease enter 1-3 or 0 to exit!\n");
			continue;
		}

		// Previous Revenue Input
		while (1) {
			printf("\nEnter Annual Revenue for %d: ", latestRevenueYear);

			if (scanf_s("%Lf", &latestAnnualRevenue) != 1) {
				printf("\nInvalid input. Please enter a long double.\n");
				return;
			}

			if (latestAnnualRevenue <= 0) {
				printf("\nPlease enter an input larger than 0!\n");
				continue;
			}

			break;
		}

		// Preceding Revenue Input
		while (1) {
			printf("Enter Annual Revenue for %d: ", precedingRevenueYear);

			if (scanf_s("%Lf", &precedingAnnualRevenue) != 1) {
				printf("\nInvalid input. Please enter a long double.\n");
				return;
			}

			if (precedingAnnualRevenue <= 0) {
				printf("\nPlease enter an input larger than 0!\n");
				continue;
			}

			break;
		}

		// Identify the growth rate and volatility rate based on the two years of revenue
		const double annualGrowthRate = findGrowthRate(latestAnnualRevenue, precedingAnnualRevenue); // How much the revenue has grown from the previous year to the current year
		const double annualVolatilityRate = findVolatilityRate(latestAnnualRevenue, precedingAnnualRevenue); // How much the revenue has fluctuated from the previous year to the current year (the uncertainties)
		flush();

		switch (choice) {
		case 1:
			OpenMPParallel(latestAnnualRevenue, annualGrowthRate, annualVolatilityRate);
			break;

		case 2:
			CUDAParallel(latestAnnualRevenue, annualGrowthRate, annualVolatilityRate);
			break;

		case 3:
			MPIParallel(latestAnnualRevenue, annualGrowthRate, annualVolatilityRate);
			break;

		}
	}
}

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);

	srand((unsigned int)time(NULL)); // Seed the random number generator

	mainMenu();

	MPI_Finalize();
	return 0;
}

// Helpers
double findGrowthRate(long double latestRevenue, long double precedingRevenue) {
	// Year over year growth from revenue2 to revenue1
	double growthRate = 0.0;

	// Growth Rate formula
	// Growth Rate = (Revenue1 - Revenue2) / Revenue2
	growthRate = static_cast<double>((latestRevenue - precedingRevenue) / precedingRevenue);

	return growthRate;
}

double findVolatilityRate(long double latestRevenue, long double precedingRevenue) {
	// Volatility is the standard deviation of the revenues
	double volatilityRate = 0.0;
	
	// Mean formula
	long double meanRevenue = (latestRevenue + precedingRevenue) / 2.0L;

	// Sample Variance formula
	long double variance = ((pow(latestRevenue - meanRevenue, 2) + pow(precedingRevenue - meanRevenue, 2)) / (2 - 1));

	// Sample standard deviation formula (Volatility)
	long double volatility = sqrtl(variance);

	// Finding the volatility in decimal (Sample standard deviation / mean)
	volatilityRate = static_cast<double>(volatility / meanRevenue);

	return volatilityRate;
}

long double oneSimulation(long double latestRevenue, double growthRate, double volatilityRate) {
	// To find the current forecasted revenue
	// Formula would be
	// Revenue = Revenue-1 * (1 + Growth Rate + (Volatility Rate * Random Fluctuation))
	// randomFluctuation is the randomized variable that would fluctuate between -2.0 to 2.0 because of market risks
	// -2 means a bad year, and 2 means a good year
	long double revenueSimulated = 0.0;
	double randomFluctuation = 0.0;

	int lower = -2;
	int upper = 2;

	// Random Fluctuation from -2 to 2
	randomFluctuation = (rand() % (upper - lower + 1)) + lower;

	revenueSimulated = latestRevenue * (1 + growthRate + (volatilityRate * randomFluctuation));

	return revenueSimulated;
}

long double monteCarloSimulation(long double revenues[NUM_OF_STEPS]) {
	// Monte Carlo Formula to find the revenue that is found
	// Formula is
	// 1/N (Sum of All Simulated Revenue in a year)
	// 1/NUM_OF_STEPS * SUM(revenues)
	long double sum = 0.0L;
	long double revenueCurrent = 0.0;

	for (long i = 0; i < NUM_OF_STEPS; i++) {
		sum += revenues[i];
	}

	// Monte Carlo Formula
	revenueCurrent = (1 / static_cast<long double>(NUM_OF_STEPS)) * sum;

	return revenueCurrent;
}

// Prompt Helpers
void flush() {
	printf("\033[2J\033[H");
	fflush(stdout);
}