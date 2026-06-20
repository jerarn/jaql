#include <jaql/core/result.hpp>

#include <benchmark/benchmark.h>

namespace {

constexpr double kSeed = 1.0;

[[nodiscard]] auto raw_chain(double x) -> double {
  x = x + 1.0;
  x = x * 2.0;
  x = x - 0.5;
  x = x / 3.0;
  x = x + x;
  return x;
}

[[nodiscard]] auto result_chain(double x) -> jaql::core::Result<double> {
  return jaql::core::ok(x)
      .map([](double value) { return value + 1.0; })
      .map([](double value) { return value * 2.0; })
      .map([](double value) { return value - 0.5; })
      .map([](double value) { return value / 3.0; })
      .map([](double value) { return value + value; });
}

}  // namespace

static void BM_RawDouble(benchmark::State& state) {
  for (auto _ : state) {
    double value = kSeed;
    benchmark::DoNotOptimize(value);
    value = raw_chain(value);
    benchmark::DoNotOptimize(value);
  }
}

static void BM_ResultChain(benchmark::State& state) {
  for (auto _ : state) {
    double value = kSeed;
    benchmark::DoNotOptimize(value);
    const auto result = result_chain(value);
    double output = *result;
    benchmark::DoNotOptimize(output);
  }
}

BENCHMARK(BM_RawDouble);
BENCHMARK(BM_ResultChain);

BENCHMARK_MAIN();
