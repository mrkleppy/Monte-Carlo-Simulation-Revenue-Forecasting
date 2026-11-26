#include <stdio.h>
#include <ctime>
#include <chrono>
#include <vector>
#include <string>
#include <random>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <curand_kernel.h>
#include "MonteCarloSimulation.h"
#include "Sequential.h"

using namespace std;

const int BLOCKS = 4; // Groups of threads, each block has n threads
const int THREADS = 4; // Groups of simulations, each thread handles n simulations
const int TOTAL_THREADS = BLOCKS * THREADS;

#define CHECK(call) do { cudaError_t e = (call); if (e != cudaSuccess) \
	printf("CUDA error line %d: %s\n", __LINE__, cudaGetErrorString(e)); } while (0) // Macro to check CUDA calls

// Runs on the GPU. Every thread handles simulations tid and tid+TOTAL_THREADS
__global__ void simulateKernel(double latest, double growth, double vol, double* sums, unsigned long long seed) // Functions for every thread to run on the GPU
{
	int tid = blockIdx.x * blockDim.x + threadIdx.x; // Unique thread ID with formala, block number * block size + thread number in block

	curandState state;							// random number generator state
	curand_init(seed, tid, 0, &state);		// each thread gets its own random stream

	double local[FORECAST_YEARS] = {};         // this thread's running total per year

	for (long long sim = tid; sim < NUM_OF_STEPS; sim += TOTAL_THREADS) { // each thread handles multiple simulations, spaced TOTAL_THREADS apart
		// 4-year revenue forecast for this simulation
		double revenue = latest;
		for (int y = 0; y < FORECAST_YEARS; ++y) {
			int r = (int)(curand(&state) % 5u) - 2;       // random -2..2
			revenue = revenue * (1.0 + growth + vol * r); // same formula as oneSimulation
			local[y] += revenue;
		}
	}

	for (int y = 0; y < FORECAST_YEARS; ++y) // write this thread's totals to global memory
		sums[y * TOTAL_THREADS + tid] = local[y];
}

static void printRow(const char* method, const ForecastResult& r)
// Print a row of the forecast table
{
	printf("%-11s", method);
	for (int y = 0; y < FORECAST_YEARS; ++y)
		printf("| RM %s%12s%s ", BLUE, formatWithCommas(r.meanRevenue[y], 2).c_str(), RESET);
	printf("\n");
}

void CUDAParallel(long double latestAnnualRevenue, double annualGrowthRate, double annualVolatilityRate)
{
	static random_device rd; // Seed for random number generator
	unsigned long long seed = rd(); // Generate a random seed for the simulation

	time_t now = time(NULL);
	tm lt = {};
	localtime_s(&lt, &now);
	int firstYear = lt.tm_year + 1900;

	// growth = (latest - preceding) / preceding, so preceding can be recovered, used for display
	long double precedingRevenue = latestAnnualRevenue / (1.0L + annualGrowthRate);

	cudaDeviceProp prop;
	cudaGetDeviceProperties(&prop, 0);

	// ---------- Sequential (timed) ----------
	auto t0 = chrono::high_resolution_clock::now(); // start timer
	ForecastResult seq = SequentialOperation(latestAnnualRevenue, annualGrowthRate, annualVolatilityRate);
	auto t1 = chrono::high_resolution_clock::now(); // stop timer
	double seqSec = chrono::duration<double>(t1 - t0).count(); // elapsed time in seconds for sequential operation

	// ---------- CUDA (timed with cudaEvent_t) ----------
	double* dSums = nullptr;
	CHECK(cudaMalloc(&dSums, sizeof(double) * FORECAST_YEARS * TOTAL_THREADS)); // Reserve space on the GPU for the sums of all threads

	// Create CUDA events for timing
	cudaEvent_t start, stop;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);

	vector<double> hSums(FORECAST_YEARS * TOTAL_THREADS); // Reserve space on the CPU for the sums of all threads

	cudaEventRecord(start); // Start timing
	// Launch the kernel with <BLOCKS> blocks and <THREADS> threads per block
	simulateKernel << <BLOCKS, THREADS >> > ((double)latestAnnualRevenue, annualGrowthRate, annualVolatilityRate, dSums, seed);
	// Check for errors during kernel launch and synchronize the device
	CHECK(cudaGetLastError());
	// Wait for the GPU to finish before accessing the results
	CHECK(cudaDeviceSynchronize());
	// Copy the results from the GPU back to the CPU
	CHECK(cudaMemcpy(hSums.data(), dSums, sizeof(double) * hSums.size(), cudaMemcpyDeviceToHost));
	cudaEventRecord(stop); // Stop timing
	cudaEventSynchronize(stop); // Wait for the stop event to complete

	// Calculate the elapsed time in milliseconds and convert to seconds
	float gpuMs = 0.0f;
	cudaEventElapsedTime(&gpuMs, start, stop);
	double gpuSec = gpuMs / 1000.0;

	// ---------- Calculate mean revenue per year ----------
	ForecastResult par = {};
	for (int y = 0; y < FORECAST_YEARS; ++y) {
		long double sum = 0.0L;
		for (int t = 0; t < TOTAL_THREADS; ++t)
			sum += hSums[y * TOTAL_THREADS + t];
		par.meanRevenue[y] = (1 / (long double)NUM_OF_STEPS) * sum;
	}

	cudaFree(dSums); // Free the GPU memory allocated for the sums
	cudaEventDestroy(start); // Destroy the CUDA start events used for timing
	cudaEventDestroy(stop); // Destroy the CUDA stop events used for timing

	// ---------- Output ----------
	printf("\n\nMonte Carlo Revenue Forecasting - CUDA\n");
	printf("----------------------------------------\n");
	printf("GPU: %s\n", prop.name);
	printf("Streaming Multiprocessors: %d\n", prop.multiProcessorCount);
	printf("CUDA Threads Launched: %d (%d blocks x %d threads)\n\n",
		TOTAL_THREADS, BLOCKS, THREADS);

	printf("Annual Revenue for Year %d: %sRM %s%s\n", firstYear - 2, MAGENTA,
		formatWithCommas(precedingRevenue, 2).c_str(), RESET);
	printf("Annual Revenue for Year %d: %sRM %s%s\n\n", firstYear - 1, MAGENTA,
		formatWithCommas(latestAnnualRevenue, 2).c_str(), RESET);

	printf("Annual Growth Rate: %s%.4f%%%s\n", LIME, annualGrowthRate * 100.0, RESET);
	printf("Annual Volatility Rate: %s%.4f%%%s\n", LIME, annualVolatilityRate * 100.0, RESET);
	printf("Number of Simulations: %s%lld%s\n\n", LIME, (long long)NUM_OF_STEPS, RESET);

	printf("Forecasted Revenues\n");
	printf("Assumption: constant growth and uncertainty parameters.\n\n");

	printf("%-11s", "Method");
	for (int y = 0; y < FORECAST_YEARS; ++y) printf("| Year %-11d", firstYear + y);
	printf("\n%s\n", string(11 + 18 * FORECAST_YEARS, '-').c_str());
	printRow("Sequential", seq);
	printRow("CUDA", par);

	printf("\nPerformance Comparison\n");
	printf("----------------------------------------\n");
	printf("Simulations (Steps): %lld\n", (long long)NUM_OF_STEPS);
	printf("CUDA threads: %d\n\n", TOTAL_THREADS);

	printf("Ts: %s%.6f seconds%s\n", ORANGE, seqSec, RESET);
	printf("Tp: %s%.6f seconds%s\n", ORANGE, gpuSec, RESET);

	double speedup = seqSec / gpuSec;
	double efficiency = speedup / static_cast<double>(TOTAL_THREADS);

	printf("Speedup: %s%.4f%s\n", LIME, seqSec / gpuSec, RESET);
	printf("Efficiency: %s%.2f%%%s\n", LIME, efficiency * 100.0, RESET);

	printf("\nPress Enter to continue...");
	int ch;
	while ((ch = getchar()) != '\n' && ch != EOF) {}   // throw away the leftover Enter from scanf_s
	getchar();                                         // now wait for a real key press
	flush();
}