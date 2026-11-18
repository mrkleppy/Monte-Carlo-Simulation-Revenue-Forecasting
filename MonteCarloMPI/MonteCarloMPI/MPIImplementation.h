#pragma once

#ifndef MPIImplementation
#define MPIImplementation
#include <mpi.h>

// Stephen
void MPIParallel(long double latestAnnualRevenue, double annualGrowthRate, double annualVolatilityRate, int firstForecastYear);

#endif