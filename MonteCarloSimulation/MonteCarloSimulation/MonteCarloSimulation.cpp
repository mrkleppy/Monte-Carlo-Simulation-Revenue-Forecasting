#include <stdio.h>
#include "OpenMP.h"
#include "CUDA.h"
#include "MPI.h"

void mainMenu() {

	while (1) {
		long double yearMinusOne = 0.0, yearMinusTwo = 0.0, yearMinusThree = 0.0;
		double volatilityRate = 0.0;
		int choice;

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

		// Year -1 Revenue Input
		while (1) {
			printf("\nEnter Revenue for Year -1: ");

			if (scanf_s("%Lf", &yearMinusOne) != 1) {
				printf("\nInvalid input. Please enter a long double.\n");
				return;
			}

			if (yearMinusOne <= 0) {
				printf("\nPlease enter an input larger than 0!\n");
				continue;
			}

			break;
		}

		// Year -2 Revenue Input
		while (1) {
			printf("Enter Revenue for Year -2: ");

			if (scanf_s("%Lf", &yearMinusTwo) != 1) {
				printf("\nInvalid input. Please enter a long double.\n");
				return;
			}

			if (yearMinusTwo <= 0) {
				printf("\nPlease enter an input larger than 0!\n");
				continue;
			}

			break;
		}

		// Year -3 Revenue Input
		while (1) {
			printf("Enter Revenue for Year -3: ");

			if (scanf_s("%Lf", &yearMinusThree) != 1) {
				printf("\nInvalid input. Please enter a long double.\n");
				return;
			}

			if (yearMinusThree <= 0) {
				printf("\nPlease enter an input larger than 0!\n");
				continue;
			}

			break;
		}

		while (1) {
			printf("Enter Volatility Rate (0.0 - 100.0): ");

			if (scanf_s("%lf", &volatilityRate) != 1) {
				printf("\nInvalid input. Please enter a double.\n");
				return;
			}

			if (volatilityRate < 0.0 || volatilityRate > 100.0) {
				printf("\nPlease enter an input between 0.0 and 100.0!\n");
				continue;
			}

			break;
		}

		switch (choice) {
		case 1:
			OpenMPOperation(yearMinusOne, yearMinusTwo, yearMinusThree, volatilityRate);
			break;

		case 2:
			CUDAOperation(yearMinusOne, yearMinusTwo, yearMinusThree, volatilityRate);
			break;

		case 3:
			MPIOperation(yearMinusOne, yearMinusTwo, yearMinusThree, volatilityRate);
			break;

		}
	}
}

int main() {

	mainMenu();

	return 0;
}