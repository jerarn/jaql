#include "jaql/greeks.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Black Scholes Greeks baseline", "[greeks]") {
    const double delta = jaql::black_scholes_delta(jaql::OptionType::Call, 100.0, 100.0, 0.05, 0.2, 1.0);
    const double gamma = jaql::black_scholes_gamma(100.0, 100.0, 0.05, 0.2, 1.0);
    const double vega = jaql::black_scholes_vega(100.0, 100.0, 0.05, 0.2, 1.0);

    REQUIRE_THAT(delta, Catch::Matchers::WithinRel(0.6368306512, 1e-8));
    REQUIRE_THAT(gamma, Catch::Matchers::WithinRel(0.0187620173, 1e-8));
    REQUIRE_THAT(vega, Catch::Matchers::WithinRel(37.5240346917, 1e-8));
}
