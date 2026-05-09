#pragma once

#include "jaql/date.hpp"

namespace jaql {

enum class BusinessDayConvention {
    Following,
    Preceding,
    ModifiedFollowing,
    Unadjusted
};

[[nodiscard]] bool is_business_day(const Date& date);
[[nodiscard]] Date add_days(const Date& date, int day_count);
[[nodiscard]] Date adjust_business_day(const Date& date, BusinessDayConvention convention);

} // namespace jaql
