#include "jaql/curves.hpp"
#include "jaql/instruments.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Forward pricing uses carry", "[forward]") {
    const jaql::Forward fwd{100.0, 100.0, 0.05, 1.0};
    REQUIRE_THAT(fwd.price(), Catch::Matchers::WithinRel(4.8770575499, 1e-9));
}

TEST_CASE("Black Scholes call baseline", "[option]") {
    const auto value = jaql::black_scholes_price(jaql::OptionType::Call, 100.0, 100.0, 0.05, 0.2, 1.0);
    REQUIRE_THAT(value, Catch::Matchers::WithinRel(10.4505835722, 1e-9));
}

TEST_CASE("Bond creates periodic cash flows", "[bond]") {
    const jaql::Bond bond(100.0, 0.06, {2025, 1, 1}, {2028, 1, 1}, 2);
    const auto flows = bond.cash_flows();
    REQUIRE(flows.size() == 6);
    REQUIRE(flows.back().amount > 100.0);
}

TEST_CASE("Yield curves discount and interpolate", "[curve]") {
    const jaql::FlatYieldCurve flat(0.03);
    REQUIRE_THAT(flat.zero_rate(5.0), Catch::Matchers::WithinRel(0.03, 1e-12));

    const jaql::BootstrappedYieldCurve boot({1.0, 2.0, 3.0}, {0.02, 0.03, 0.04});
    REQUIRE_THAT(boot.zero_rate(1.5), Catch::Matchers::WithinRel(0.025, 1e-12));
}
