#pragma once

#ifndef SEQUENTIAL
#define SEQUENTIAL
#include "MonteCarloSimulation.h"

// Functions
void SequentialOperation(long double latestAnnualRevenue, double annualGrowthRate, double annualVolatilityRate);

// USE THIS FOR FUTURE
//ForecastResult SequentialOperation(long double latestAnnualRevenue, double annualGrowthRate, double annualVolatilityRate);

#endif 