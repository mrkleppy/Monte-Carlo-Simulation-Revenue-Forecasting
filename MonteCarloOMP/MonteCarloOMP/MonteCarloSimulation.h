#pragma once

#ifndef MonteCarloSimulation
#define MonteCarloSimulation

#include <string>

#define BLUE "\x1b[0;34m"
#define MAGENTA "\033[35m"
#define LIME "\033[1;32m"
#define ORANGE "\033[38;5;208m"
#define RESET "\x1b[0m"
#define NUM_OF_STEPS 1000000
#define FORECAST_YEARS 4

using namespace std;

// Structs
struct ForecastResult {
	long double meanRevenue[FORECAST_YEARS] = {};
};

// Functions
void mainMenu();

// Helpers
double findGrowthRate(long double revenue1, long double revenue2);
double findVolatilityRate(long double revenue1, long double revenue2);
long double oneSimulation(long double revenueMinusOne, double growthRate, double volatilityRate);

// Prompt Helpers
void flush();
string formatWithCommas(long double value, int decimalPlaces = 2);

#endif