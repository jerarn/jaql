#pragma once

#include "jaql/date.hpp"
#include "jaql/day_count.hpp"

#include <vector>

namespace jaql {

enum class OptionType {
    Call,
    Put
};

struct CashFlow {
    Date payment_date;
    double amount;
};

class Bond {
public:
    Bond(double face, double coupon_rate, Date issue_date, Date maturity_date, int payments_per_year = 2);

    [[nodiscard]] std::vector<CashFlow> cash_flows() const;
    [[nodiscard]] double price(double yield_rate, DayCountConvention convention = DayCountConvention::Actual365Fixed) const;

private:
    double face_;
    double coupon_rate_;
    Date issue_date_;
    Date maturity_date_;
    int payments_per_year_;
};

struct Forward {
    double spot;
    double strike;
    double rate;
    double time_to_maturity;

    [[nodiscard]] double price() const;
};

[[nodiscard]] double black_scholes_price(
    OptionType type,
    double spot,
    double strike,
    double rate,
    double volatility,
    double time_to_maturity);

} // namespace jaql
