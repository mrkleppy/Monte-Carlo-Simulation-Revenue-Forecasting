#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <locale>
#include <ctime>
#include <cmath>
#include <math.h>

#include "MonteCarloSimulation.h"

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

// Prompt Helpers
void flush() {
	printf("\033[2J\033[H");
	fflush(stdout);
}

string formatWithCommas(long double value, int decimalPlaces) {
	ostringstream stream;

	stream.imbue(locale::classic());
	stream << fixed << setprecision(decimalPlaces) << value;
	string formatted = stream.str();
	size_t decimalPosition = formatted.find('.');
	size_t position = decimalPosition == string::npos ? formatted.length() : decimalPosition;
	size_t firstDigit = !formatted.empty() && formatted[0] == '-' ? 1 : 0;

	while (position > firstDigit + 3) {
		position -= 3;
		formatted.insert(position, ",");
	}

	return formatted;
}