#include "jaql/curves.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace jaql {

FlatYieldCurve::FlatYieldCurve(double rate)
    : rate_(rate) {}

double FlatYieldCurve::discount(double time) const {
    return std::exp(-rate_ * time);
}

double FlatYieldCurve::zero_rate(double /*time*/) const {
    return rate_;
}

BootstrappedYieldCurve::BootstrappedYieldCurve(std::vector<double> times, std::vector<double> zero_rates)
    : times_(std::move(times))
    , zero_rates_(std::move(zero_rates)) {
    if (times_.empty() || times_.size() != zero_rates_.size()) {
        throw std::invalid_argument("times and zero_rates must have matching non-empty sizes");
    }
    if (!std::is_sorted(times_.begin(), times_.end())) {
        throw std::invalid_argument("times must be sorted ascending");
    }
}

double BootstrappedYieldCurve::discount(double time) const {
    const double rate = zero_rate(time);
    return std::exp(-rate * time);
}

double BootstrappedYieldCurve::zero_rate(double time) const {
    if (time <= times_.front()) {
        return zero_rates_.front();
    }
    if (time >= times_.back()) {
        return zero_rates_.back();
    }

    const auto upper = std::upper_bound(times_.begin(), times_.end(), time);
    const auto lower = std::prev(upper);
    const std::size_t lower_idx = static_cast<std::size_t>(std::distance(times_.begin(), lower));
    const std::size_t upper_idx = static_cast<std::size_t>(std::distance(times_.begin(), upper));

    const double t1 = times_[lower_idx];
    const double t2 = times_[upper_idx];
    const double r1 = zero_rates_[lower_idx];
    const double r2 = zero_rates_[upper_idx];

    const double w = (time - t1) / (t2 - t1);
    return r1 + (r2 - r1) * w;
}

} // namespace jaql
