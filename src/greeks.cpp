#include "jaql/greeks.hpp"

#include <cmath>

namespace jaql {

namespace {

[[nodiscard]] double normal_pdf(double x) {
    constexpr double inv_sqrt_2pi = 0.39894228040143267794;
    return inv_sqrt_2pi * std::exp(-0.5 * x * x);
}

[[nodiscard]] double normal_cdf(double x) {
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

[[nodiscard]] double d1(double spot, double strike, double rate, double volatility, double time_to_maturity) {
    return (std::log(spot / strike) + (rate + 0.5 * volatility * volatility) * time_to_maturity) / (volatility * std::sqrt(time_to_maturity));
}

[[nodiscard]] double d2(double spot, double strike, double rate, double volatility, double time_to_maturity) {
    return d1(spot, strike, rate, volatility, time_to_maturity) - volatility * std::sqrt(time_to_maturity);
}

} // namespace

double black_scholes_delta(OptionType type, double spot, double strike, double rate, double volatility, double time_to_maturity) {
    const double value = normal_cdf(d1(spot, strike, rate, volatility, time_to_maturity));
    if (type == OptionType::Call) {
        return value;
    }
    return value - 1.0;
}

double black_scholes_gamma(double spot, double strike, double rate, double volatility, double time_to_maturity) {
    const double d1_value = d1(spot, strike, rate, volatility, time_to_maturity);
    return normal_pdf(d1_value) / (spot * volatility * std::sqrt(time_to_maturity));
}

double black_scholes_vega(double spot, double strike, double rate, double volatility, double time_to_maturity) {
    const double d1_value = d1(spot, strike, rate, volatility, time_to_maturity);
    return spot * normal_pdf(d1_value) * std::sqrt(time_to_maturity);
}

double black_scholes_theta(OptionType type, double spot, double strike, double rate, double volatility, double time_to_maturity) {
    const double d1_value = d1(spot, strike, rate, volatility, time_to_maturity);
    const double d2_value = d2(spot, strike, rate, volatility, time_to_maturity);
    const double first_term = -(spot * normal_pdf(d1_value) * volatility) / (2.0 * std::sqrt(time_to_maturity));

    if (type == OptionType::Call) {
        return first_term - rate * strike * std::exp(-rate * time_to_maturity) * normal_cdf(d2_value);
    }
    return first_term + rate * strike * std::exp(-rate * time_to_maturity) * normal_cdf(-d2_value);
}

double black_scholes_rho(OptionType type, double spot, double strike, double rate, double volatility, double time_to_maturity) {
    const double d2_value = d2(spot, strike, rate, volatility, time_to_maturity);
    if (type == OptionType::Call) {
        return strike * time_to_maturity * std::exp(-rate * time_to_maturity) * normal_cdf(d2_value);
    }
    return -strike * time_to_maturity * std::exp(-rate * time_to_maturity) * normal_cdf(-d2_value);
}

} // namespace jaql
