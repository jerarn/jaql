#include "jaql/calendar.hpp"
#include "jaql/curves.hpp"
#include "jaql/day_count.hpp"
#include "jaql/greeks.hpp"
#include "jaql/instruments.hpp"
#include "jaql/monte_carlo.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace jaql;

PYBIND11_MODULE(jaql, m) {
    py::class_<Date>(m, "Date")
        .def(py::init<int, unsigned int, unsigned int>())
        .def_readwrite("year", &Date::year)
        .def_readwrite("month", &Date::month)
        .def_readwrite("day", &Date::day)
        .def("to_string", &Date::to_string);

    py::enum_<DayCountConvention>(m, "DayCountConvention")
        .value("Actual360", DayCountConvention::Actual360)
        .value("Actual365Fixed", DayCountConvention::Actual365Fixed)
        .value("Thirty360US", DayCountConvention::Thirty360US);

    py::enum_<BusinessDayConvention>(m, "BusinessDayConvention")
        .value("Following", BusinessDayConvention::Following)
        .value("Preceding", BusinessDayConvention::Preceding)
        .value("ModifiedFollowing", BusinessDayConvention::ModifiedFollowing)
        .value("Unadjusted", BusinessDayConvention::Unadjusted);

    py::enum_<OptionType>(m, "OptionType")
        .value("Call", OptionType::Call)
        .value("Put", OptionType::Put);

    py::class_<CashFlow>(m, "CashFlow")
        .def_readwrite("payment_date", &CashFlow::payment_date)
        .def_readwrite("amount", &CashFlow::amount);

    py::class_<Bond>(m, "Bond")
        .def(py::init<double, double, Date, Date, int>(), py::arg("face"), py::arg("coupon_rate"), py::arg("issue_date"), py::arg("maturity_date"), py::arg("payments_per_year") = 2)
        .def("cash_flows", &Bond::cash_flows)
        .def("price", &Bond::price, py::arg("yield_rate"), py::arg("convention") = DayCountConvention::Actual365Fixed);

    py::class_<Forward>(m, "Forward")
        .def(py::init<>())
        .def_readwrite("spot", &Forward::spot)
        .def_readwrite("strike", &Forward::strike)
        .def_readwrite("rate", &Forward::rate)
        .def_readwrite("time_to_maturity", &Forward::time_to_maturity)
        .def("price", &Forward::price);

    py::class_<YieldCurve>(m, "YieldCurve");

    py::class_<FlatYieldCurve, YieldCurve>(m, "FlatYieldCurve")
        .def(py::init<double>())
        .def("discount", &FlatYieldCurve::discount)
        .def("zero_rate", &FlatYieldCurve::zero_rate);

    py::class_<BootstrappedYieldCurve, YieldCurve>(m, "BootstrappedYieldCurve")
        .def(py::init<std::vector<double>, std::vector<double>>())
        .def("discount", &BootstrappedYieldCurve::discount)
        .def("zero_rate", &BootstrappedYieldCurve::zero_rate);

    m.def("days_between", &days_between);
    m.def("year_fraction", &year_fraction);
    m.def("is_business_day", &is_business_day);
    m.def("adjust_business_day", &adjust_business_day);
    m.def("black_scholes_price", &black_scholes_price);
    m.def("black_scholes_delta", &black_scholes_delta);
    m.def("black_scholes_gamma", &black_scholes_gamma);
    m.def("black_scholes_vega", &black_scholes_vega);
    m.def("black_scholes_theta", &black_scholes_theta);
    m.def("black_scholes_rho", &black_scholes_rho);
    m.def("black_scholes_monte_carlo_price", &black_scholes_monte_carlo_price,
          py::arg("type"), py::arg("spot"), py::arg("strike"), py::arg("rate"), py::arg("volatility"), py::arg("time_to_maturity"), py::arg("paths"), py::arg("seed") = 42);
}
