#include "jaql/instruments.hpp"

#include <benchmark/benchmark.h>

static void BM_BlackScholesCall(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(jaql::black_scholes_price(jaql::OptionType::Call, 100.0, 100.0, 0.05, 0.2, 1.0));
    }
}

BENCHMARK(BM_BlackScholesCall);
BENCHMARK_MAIN();
