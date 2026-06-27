#include <jaql/core/date.hpp>

#include <jaql/core/error.hpp>
#include <jaql/core/result.hpp>

#include <algorithm>
#include <array>
#include <format>

namespace jaql::core {

namespace {

/// @brief Converts a year/month/day to days since 1970-01-01.
///
/// Implements Howard Hinnant's branch-free proleptic Gregorian algorithm.
auto days_from_civil(int year, int month, int day) noexcept -> int {
  year -= month <= 2 ? 1 : 0;
  const int era = (year >= 0 ? year : year - 399) / 400;
  const int yoe = year - era * 400;
  const int shifted_month = month > 2 ? month - 3 : month + 9;
  const int doy = (153 * shifted_month + 2) / 5 + day - 1;
  const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + doe - 719468;
}

struct YearMonthDay {
  int year;
  int month;
  int day;
};

/// @brief Converts days since 1970-01-01 to a year/month/day triple.
///
/// Inverse of @c days_from_civil using Howard Hinnant's algorithm.
auto civil_from_days(int serial) noexcept -> YearMonthDay {
  serial += 719468;
  const int era = (serial >= 0 ? serial : serial - 146096) / 146097;
  const int doe = serial - era * 146097;
  const int yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const int year = yoe + era * 400;
  const int doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const int shifted_month = (5 * doy + 2) / 153;
  const int day = doy - (153 * shifted_month + 2) / 5 + 1;
  const int month = shifted_month < 10 ? shifted_month + 3 : shifted_month - 9;
  return {year + (month <= 2 ? 1 : 0), month, day};
}

}  // namespace

auto is_leap_year(int year) -> bool {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

auto days_in_month(int month, int year) -> int {
  static constexpr std::array<int, 13> lengths{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month < 1 || month > 12) {
    return 0;
  }
  if (month == 2 && is_leap_year(year)) {
    return 29;
  }
  return lengths.at(static_cast<std::size_t>(month));
}

auto Date::from_ymd(int year, int month, int day) -> Result<Date> {
  if (month < 1 || month > 12) {
    return err<Code::InvalidDate, Date>(std::format("month {} out of range [1, 12]", month));
  }
  if (day < 1 || day > days_in_month(month, year)) {
    return err<Code::InvalidDate, Date>(
        std::format("day {} out of range for {:04}-{:02}", day, year, month));
  }
  return Date{days_from_civil(year, month, day)};
}

auto Date::from_serial(int serial) noexcept -> Date { return Date{serial}; }

auto Date::year() const -> int { return civil_from_days(serial_).year; }

auto Date::month() const -> int { return civil_from_days(serial_).month; }

auto Date::day() const -> int { return civil_from_days(serial_).day; }

auto Date::day_of_week() const -> DayOfWeek {
  // Serial 0 (1970-01-01) is a Thursday, whose index in DayOfWeek is 3.
  const int index = ((serial_ % 7) + 7 + 3) % 7;
  return static_cast<DayOfWeek>(index);
}

auto Date::add_days(int days) const noexcept -> Date { return Date{serial_ + days}; }

auto Date::add_months(int months) const -> Date {
  const YearMonthDay ymd = civil_from_days(serial_);
  const int total = ymd.year * 12 + (ymd.month - 1) + months;
  const int new_year = total >= 0 ? total / 12 : (total - 11) / 12;
  const int new_month = total - new_year * 12 + 1;
  const int new_day = std::min(ymd.day, days_in_month(new_month, new_year));
  return Date{days_from_civil(new_year, new_month, new_day)};
}

auto Date::add_years(int years) const -> Date {
  const YearMonthDay ymd = civil_from_days(serial_);
  const int new_year = ymd.year + years;
  const int new_day = std::min(ymd.day, days_in_month(ymd.month, new_year));
  return Date{days_from_civil(new_year, ymd.month, new_day)};
}

auto WeekendCalendar::is_business_day(const Date& date) const -> bool {
  const DayOfWeek dow = date.day_of_week();
  return dow != DayOfWeek::Saturday && dow != DayOfWeek::Sunday;
}

namespace {

auto roll_following(const Date& date, const Calendar& calendar) -> Date {
  Date result = date;
  while (!calendar.is_business_day(result)) {
    result = result.add_days(1);
  }
  return result;
}

auto roll_preceding(const Date& date, const Calendar& calendar) -> Date {
  Date result = date;
  while (!calendar.is_business_day(result)) {
    result = result.add_days(-1);
  }
  return result;
}

}  // namespace

auto adjust(const Date& date, BusinessDayConvention convention, const Calendar& calendar) -> Date {
  switch (convention) {
    case BusinessDayConvention::Unadjusted:
      return date;
    case BusinessDayConvention::Following:
      return roll_following(date, calendar);
    case BusinessDayConvention::Preceding:
      return roll_preceding(date, calendar);
    case BusinessDayConvention::ModifiedFollowing: {
      const Date following = roll_following(date, calendar);
      return following.month() == date.month() ? following : roll_preceding(date, calendar);
    }
    case BusinessDayConvention::ModifiedPreceding: {
      const Date preceding = roll_preceding(date, calendar);
      return preceding.month() == date.month() ? preceding : roll_following(date, calendar);
    }
  }
  return date;
}

auto add_business_days(const Date& date, int count, const Calendar& calendar) -> Date {
  const int step = count >= 0 ? 1 : -1;
  Date result = date;
  for (int remaining = count; remaining != 0; remaining -= step) {
    result = result.add_days(step);
    while (!calendar.is_business_day(result)) {
      result = result.add_days(step);
    }
  }
  return result;
}

auto business_days_between(const Date& start, const Date& end, const Calendar& calendar) -> int {
  const Date first = std::min(start, end);
  const Date last = std::max(start, end);
  int count = 0;
  for (Date current = first; current < last; current = current.add_days(1)) {
    if (calendar.is_business_day(current)) {
      ++count;
    }
  }
  return start <= end ? count : -count;
}

}  // namespace jaql::core
