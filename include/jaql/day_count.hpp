#pragma once

#include "jaql/date.hpp"

namespace jaql {

enum class DayCountConvention {
    Actual360,
    Actual365Fixed,
    Thirty360US
};

[[nodiscard]] double year_fraction(const Date& start, const Date& end, DayCountConvention convention);

} // namespace jaql
