#include "jaql/monte_carlo.hpp"

#include <cmath>
#include <random>

namespace jaql {

double black_scholes_monte_carlo_price(
    OptionType type,
    double spot,
    double strike,
    double rate,
    double volatility,
    double time_to_maturity,
    std::size_t paths,
    unsigned int seed) {
    std::mt19937 generator(seed);
    std::normal_distribution<> normal(0.0, 1.0);

    const double drift = (rate - 0.5 * volatility * volatility) * time_to_maturity;
    const double diffusion = volatility * std::sqrt(time_to_maturity);

    double payoff_sum = 0.0;
    for (std::size_t i = 0; i < paths; ++i) {
        const double terminal = spot * std::exp(drift + diffusion * normal(generator));
        const double payoff = (type == OptionType::Call)
            ? std::max(terminal - strike, 0.0)
            : std::max(strike - terminal, 0.0);
        payoff_sum += payoff;
    }

    return std::exp(-rate * time_to_maturity) * (payoff_sum / static_cast<double>(paths));
}

} // namespace jaql
