#include "jaql/calendar.hpp"

namespace jaql {

namespace {

Date from_sys_days(const std::chrono::sys_days& day) {
    const std::chrono::year_month_day ymd{day};
    return Date{int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day())};
}

} // namespace

bool is_business_day(const Date& date) {
    const std::chrono::weekday wd{date.to_sys_days()};
    return wd != std::chrono::Saturday && wd != std::chrono::Sunday;
}

Date add_days(const Date& date, int day_count) {
    return from_sys_days(date.to_sys_days() + std::chrono::days(day_count));
}

Date adjust_business_day(const Date& date, BusinessDayConvention convention) {
    if (convention == BusinessDayConvention::Unadjusted || is_business_day(date)) {
        return date;
    }

    if (convention == BusinessDayConvention::Following || convention == BusinessDayConvention::ModifiedFollowing) {
        Date adjusted = date;
        while (!is_business_day(adjusted)) {
            adjusted = add_days(adjusted, 1);
        }

        if (convention == BusinessDayConvention::ModifiedFollowing && adjusted.month != date.month) {
            adjusted = date;
            while (!is_business_day(adjusted)) {
                adjusted = add_days(adjusted, -1);
            }
        }

        return adjusted;
    }

    Date adjusted = date;
    while (!is_business_day(adjusted)) {
        adjusted = add_days(adjusted, -1);
    }
    return adjusted;
}

} // namespace jaql
