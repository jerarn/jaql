#include "jaql/calendar.hpp"
#include "jaql/day_count.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Day count conventions work", "[day_count]") {
    const jaql::Date start{2024, 1, 1};
    const jaql::Date end{2025, 1, 1};

    REQUIRE_THAT(jaql::year_fraction(start, end, jaql::DayCountConvention::Actual365Fixed), Catch::Matchers::WithinRel(366.0 / 365.0, 1e-12));
    REQUIRE_THAT(jaql::year_fraction(start, end, jaql::DayCountConvention::Actual360), Catch::Matchers::WithinRel(366.0 / 360.0, 1e-12));
    REQUIRE_THAT(jaql::year_fraction(start, end, jaql::DayCountConvention::Thirty360US), Catch::Matchers::WithinRel(1.0, 1e-12));
}

TEST_CASE("Business day adjustment handles weekends", "[calendar]") {
    const jaql::Date saturday{2026, 5, 9};

    const auto following = jaql::adjust_business_day(saturday, jaql::BusinessDayConvention::Following);
    REQUIRE(following.to_string() == "2026-05-11");

    const auto preceding = jaql::adjust_business_day(saturday, jaql::BusinessDayConvention::Preceding);
    REQUIRE(preceding.to_string() == "2026-05-08");
}
