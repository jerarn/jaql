#pragma once

#include <vector>

namespace jaql {

class YieldCurve {
public:
    virtual ~YieldCurve() = default;
    [[nodiscard]] virtual double discount(double time) const = 0;
    [[nodiscard]] virtual double zero_rate(double time) const = 0;
};

class FlatYieldCurve final : public YieldCurve {
public:
    explicit FlatYieldCurve(double rate);

    [[nodiscard]] double discount(double time) const override;
    [[nodiscard]] double zero_rate(double time) const override;

private:
    double rate_;
};

class BootstrappedYieldCurve final : public YieldCurve {
public:
    BootstrappedYieldCurve(std::vector<double> times, std::vector<double> zero_rates);

    [[nodiscard]] double discount(double time) const override;
    [[nodiscard]] double zero_rate(double time) const override;

private:
    std::vector<double> times_;
    std::vector<double> zero_rates_;
};

} // namespace jaql
