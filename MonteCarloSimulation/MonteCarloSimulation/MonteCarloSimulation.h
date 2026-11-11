#pragma once

#ifndef MonteCarloSimulation
#define MonteCarloSimulation

#define BLU "\x1b[0;34m"
#define RESET "\x1b[0m"
#define NUM_OF_STEPS 16

void mainMenu();

// Helpers
double findGrowthRate(long double revenue1, long double revenue2);
double findVolatilityRate(long double revenue1, long double revenue2);
long double oneSimulation(long double revenueMinusOne, double growthRate, double volatilityRate);
long double monteCarloSimulation(long double revenues[NUM_OF_STEPS]);

// Prompt Helpers
void flush();

#endif