#pragma once

#include "jaql/instruments.hpp"

#include <cstddef>

namespace jaql {

[[nodiscard]] double black_scholes_monte_carlo_price(
    OptionType type,
    double spot,
    double strike,
    double rate,
    double volatility,
    double time_to_maturity,
    std::size_t paths,
    unsigned int seed = 42);

} // namespace jaql
