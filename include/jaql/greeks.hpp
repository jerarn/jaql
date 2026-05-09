#pragma once

#include "jaql/instruments.hpp"

namespace jaql {

[[nodiscard]] double black_scholes_delta(OptionType type, double spot, double strike, double rate, double volatility, double time_to_maturity);
[[nodiscard]] double black_scholes_gamma(double spot, double strike, double rate, double volatility, double time_to_maturity);
[[nodiscard]] double black_scholes_vega(double spot, double strike, double rate, double volatility, double time_to_maturity);
[[nodiscard]] double black_scholes_theta(OptionType type, double spot, double strike, double rate, double volatility, double time_to_maturity);
[[nodiscard]] double black_scholes_rho(OptionType type, double spot, double strike, double rate, double volatility, double time_to_maturity);

} // namespace jaql
