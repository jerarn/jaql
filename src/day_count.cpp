#include "jaql/day_count.hpp"

#include <stdexcept>

namespace jaql {

namespace {

double thirty_360_us(const Date& start, const Date& end) {
    const int d1 = start.day == 31 ? 30 : static_cast<int>(start.day);
    int d2 = static_cast<int>(end.day);
    if (d1 == 30 && d2 == 31) {
        d2 = 30;
    }

    return (360.0 * (end.year - start.year) + 30.0 * (static_cast<int>(end.month) - static_cast<int>(start.month)) + (d2 - d1)) / 360.0;
}

} // namespace

double year_fraction(const Date& start, const Date& end, DayCountConvention convention) {
    switch (convention) {
    case DayCountConvention::Actual360:
        return static_cast<double>(days_between(start, end)) / 360.0;
    case DayCountConvention::Actual365Fixed:
        return static_cast<double>(days_between(start, end)) / 365.0;
    case DayCountConvention::Thirty360US:
        return thirty_360_us(start, end);
    default:
        throw std::invalid_argument("Unsupported day count convention");
    }
}

} // namespace jaql
