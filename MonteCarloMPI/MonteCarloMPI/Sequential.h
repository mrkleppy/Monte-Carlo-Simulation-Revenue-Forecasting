#pragma once

#ifndef SEQUENTIAL
#define SEQUENTIAL
#include "MonteCarloSimulation.h"

// Functions
ForecastResult SequentialOperation(long double latestAnnualRevenue, double annualGrowthRate, double annualVolatilityRate);

#endif 