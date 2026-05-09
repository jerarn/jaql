#include "jaql/instruments.hpp"
#include "jaql/calendar.hpp"

#include <cmath>
#include <stdexcept>

namespace jaql {

namespace {

constexpr double pi = 3.14159265358979323846;

[[nodiscard]] double normal_cdf(double x) {
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

} // namespace

Bond::Bond(double face, double coupon_rate, Date issue_date, Date maturity_date, int payments_per_year)
    : face_(face)
    , coupon_rate_(coupon_rate)
    , issue_date_(issue_date)
    , maturity_date_(maturity_date)
    , payments_per_year_(payments_per_year) {
    if (face_ <= 0.0 || payments_per_year_ <= 0) {
        throw std::invalid_argument("Invalid bond parameters");
    }
}

std::vector<CashFlow> Bond::cash_flows() const {
    const int total_days = days_between(issue_date_, maturity_date_);
    if (total_days <= 0) {
        return {};
    }

    const int periods = std::max(1, static_cast<int>(std::round(year_fraction(issue_date_, maturity_date_, DayCountConvention::Actual365Fixed) * payments_per_year_)));
    const double coupon = face_ * coupon_rate_ / static_cast<double>(payments_per_year_);

    std::vector<CashFlow> flows;
    flows.reserve(static_cast<std::size_t>(periods));

    Date payment_date = issue_date_;
    const int step_days = static_cast<int>(std::round(365.0 / payments_per_year_));
    for (int i = 0; i < periods; ++i) {
        payment_date = add_days(payment_date, step_days);
        if (days_between(payment_date, maturity_date_) < 0 || i == periods - 1) {
            payment_date = maturity_date_;
        }

        double amount = coupon;
        if (i == periods - 1) {
            amount += face_;
        }
        flows.push_back(CashFlow{payment_date, amount});
    }

    return flows;
}

double Bond::price(double yield_rate, DayCountConvention convention) const {
    const auto flows = cash_flows();
    double pv = 0.0;
    for (const auto& flow : flows) {
        const double t = year_fraction(issue_date_, flow.payment_date, convention);
        pv += flow.amount * std::exp(-yield_rate * t);
    }
    return pv;
}

double Forward::price() const {
    return spot - strike * std::exp(-rate * time_to_maturity);
}

double black_scholes_price(
    OptionType type,
    double spot,
    double strike,
    double rate,
    double volatility,
    double time_to_maturity) {
    if (spot <= 0.0 || strike <= 0.0 || volatility <= 0.0 || time_to_maturity <= 0.0) {
        throw std::invalid_argument("Invalid Black-Scholes input");
    }

    const double sqrt_t = std::sqrt(time_to_maturity);
    const double d1 = (std::log(spot / strike) + (rate + 0.5 * volatility * volatility) * time_to_maturity) / (volatility * sqrt_t);
    const double d2 = d1 - volatility * sqrt_t;

    if (type == OptionType::Call) {
        return spot * normal_cdf(d1) - strike * std::exp(-rate * time_to_maturity) * normal_cdf(d2);
    }
    return strike * std::exp(-rate * time_to_maturity) * normal_cdf(-d2) - spot * normal_cdf(-d1);
}

} // namespace jaql
