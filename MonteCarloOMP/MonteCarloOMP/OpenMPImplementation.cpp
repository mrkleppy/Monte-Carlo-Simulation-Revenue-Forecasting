#include <stdio.h>
#include <omp.h>
#include <random>

#include "MonteCarloSimulation.h"
#include "Sequential.h"

const int OPENMP_THREADS = 16;

static void printRow(const char* method, const ForecastResult& r)
{
    printf("%-11s", method);

    for (int y = 0; y < FORECAST_YEARS; ++y) {
        printf("| RM %s%12s%s ",
            BLUE,
            formatWithCommas(r.meanRevenue[y], 2).c_str(),
            RESET);
    }

    printf("\n");
}

void OpenMPParallel(long double latestAnnualRevenue, double annualGrowthRate, double annualVolatilityRate, int firstForecastYear) {
	const int steps = static_cast<int>(NUM_OF_STEPS);

	long double precedingRevenue = latestAnnualRevenue / (1 + annualGrowthRate); // Calculate the preceding revenue based on the latest revenue and growth rate

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

#pragma omp parallel num_threads(OPENMP_THREADS)
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
	 printf("\n\nMonte Carlo Revenue Forecasting - OpenMP\n");
    printf("----------------------------------------\n");
    printf("Logical Processors: %d\n", omp_get_num_procs());
    printf("OpenMP Threads Used: %d\n\n", threadsUsed);

    printf("Annual Revenue for Year %d: %sRM %s%s\n",
           firstForecastYear - 2, MAGENTA,
           formatWithCommas(precedingRevenue, 2).c_str(), RESET);

    printf("Annual Revenue for Year %d: %sRM %s%s\n\n",
           firstForecastYear - 1, MAGENTA,
           formatWithCommas(latestAnnualRevenue, 2).c_str(), RESET);

    printf("Annual Growth Rate: %s%.4f%%%s\n",
           LIME, annualGrowthRate * 100.0, RESET);
    printf("Annual Volatility Rate: %s%.4f%%%s\n",
           LIME, annualVolatilityRate * 100.0, RESET);
    printf("Number of Simulations: %s%d%s\n\n", LIME, steps, RESET);

    printf("Forecasted Revenues\n");
    printf("Assumption: constant growth and uncertainty parameters.\n\n");

    printf("%-11s", "Method");
    for (int y = 0; y < FORECAST_YEARS; ++y) {
        printf("| Year %-11d", firstForecastYear + y);
    }

    printf("\n%s\n", std::string(11 + 18 * FORECAST_YEARS, '-').c_str());
    printRow("Sequential", sequentialResult);
    printRow("OpenMP", parallelResult);

    printf("\nPerformance Comparison\n");
    printf("----------------------------------------\n");
    printf("Simulations (Steps): %d\n", steps);
    printf("Forecast years per scenario: %d\n", FORECAST_YEARS);
    printf("OpenMP threads: %d\n\n", threadsUsed);

    printf("Ts: %s%.6f seconds%s\n", ORANGE, Ts, RESET);
    printf("Tp: %s%.6f seconds%s\n", ORANGE, Tp, RESET);

    if (Tp > 0.0) {
        double speedup = Ts / Tp;
        double efficiency = speedup / static_cast<double>(threadsUsed);

        printf("Speedup: %s%.4f%s\n", LIME, speedup, RESET);
        printf("Efficiency: %s%.2f%%%s\n", LIME, efficiency * 100.0, RESET);
    }
    else {
        printf("Runtime is too small to calculate speedup reliably.\n");
    }

    printf("\nPress Enter to continue...");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}  // discard leftover Enter from scanf_s
    getchar();                                        // wait for a real key press
    flush();
}