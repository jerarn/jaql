#pragma once

#include <jaql/core/result.hpp>

#include <compare>

namespace jaql::core {

/// @brief Day of the week, ordered Monday through Sunday (ISO 8601 ordering).
enum class DayOfWeek {
  /// @brief Monday.
  Monday,
  /// @brief Tuesday.
  Tuesday,
  /// @brief Wednesday.
  Wednesday,
  /// @brief Thursday.
  Thursday,
  /// @brief Friday.
  Friday,
  /// @brief Saturday.
  Saturday,
  /// @brief Sunday.
  Sunday,
};

/// @brief Rule for shifting a date that falls on a non-business day.
enum class BusinessDayConvention {
  /// @brief Leave the date unchanged.
  Unadjusted,
  /// @brief Move forward to the next business day.
  Following,
  /// @brief Move forward, unless that crosses a month boundary; then move backward.
  ModifiedFollowing,
  /// @brief Move backward to the previous business day.
  Preceding,
  /// @brief Move backward, unless that crosses a month boundary; then move forward.
  ModifiedPreceding,
};

/// @brief Returns whether @p year is a leap year in the proleptic Gregorian calendar.
///
/// @param year  Gregorian calendar year.
/// @return      @c true if @p year is a leap year.
[[nodiscard]] auto is_leap_year(int year) -> bool;

/// @brief Returns the number of days in @p month of @p year.
///
/// @param month  Month in the range [1, 12].
/// @param year   Gregorian calendar year (used for February in leap years).
/// @return       Number of days in the month, or 0 if @p month is out of range.
[[nodiscard]] auto days_in_month(int month, int year) -> int;

/// @brief A calendar date backed by an integer serial number for fast arithmetic.
///
/// The serial number counts days since the Unix epoch (1970-01-01). Conversion to and
/// from year/month/day uses a proleptic Gregorian algorithm with no Julian calendar.
class Date {
 public:
  /// @brief Constructs a date from a year, month, and day, validating the calendar date.
  ///
  /// @param year   Gregorian calendar year.
  /// @param month  Month in the range [1, 12].
  /// @param day    Day of month, validated against the length of @p month.
  /// @return       The constructed date, or an @c InvalidDate error.
  [[nodiscard]] static auto from_ymd(int year, int month, int day) -> Result<Date>;

  /// @brief Constructs a date directly from a serial number, without validation.
  ///
  /// @param serial  Days since the Unix epoch (1970-01-01).
  /// @return        The corresponding date.
  [[nodiscard]] static auto from_serial(int serial) noexcept -> Date;

  /// @brief Returns the Gregorian calendar year.
  /// @return Four-digit calendar year.
  [[nodiscard]] auto year() const -> int;

  /// @brief Returns the month in the range [1, 12].
  /// @return Month number in the range [1, 12].
  [[nodiscard]] auto month() const -> int;

  /// @brief Returns the day of the month.
  /// @return Day of month in the range [1, 31].
  [[nodiscard]] auto day() const -> int;

  /// @brief Returns the serial number (days since 1970-01-01).
  /// @return Days since the Unix epoch.
  [[nodiscard]] auto serial() const noexcept -> int { return serial_; }

  /// @brief Returns the day of the week.
  /// @return Day of the week for this date.
  [[nodiscard]] auto day_of_week() const -> DayOfWeek;

  /// @brief Returns a new date shifted by @p days calendar days.
  ///
  /// @param days  Number of days to add; may be negative.
  /// @return      The shifted date.
  [[nodiscard]] auto add_days(int days) const noexcept -> Date;

  /// @brief Returns a new date shifted by @p months, clamping to month end.
  ///
  /// January 31 plus one month yields February 28 or 29.
  ///
  /// @param months  Number of months to add; may be negative.
  /// @return        The shifted date.
  [[nodiscard]] auto add_months(int months) const -> Date;

  /// @brief Returns a new date shifted by @p years, clamping leap days.
  ///
  /// February 29 plus one year yields February 28 on a non-leap target year.
  ///
  /// @param years  Number of years to add; may be negative.
  /// @return       The shifted date.
  [[nodiscard]] auto add_years(int years) const -> Date;

  /// @brief Compares two dates by serial number.
  [[nodiscard]] friend auto operator<=>(const Date& lhs, const Date& rhs) noexcept = default;

  /// @brief Returns the signed number of days from @p rhs to @p lhs.
  /// @param lhs  Left-hand date.
  /// @param rhs  Right-hand date.
  /// @return     @p lhs.serial() minus @p rhs.serial().
  [[nodiscard]] friend auto operator-(const Date& lhs, const Date& rhs) noexcept -> int {
    return lhs.serial_ - rhs.serial_;
  }

 private:
  explicit constexpr Date(int serial) noexcept : serial_(serial) {}

  int serial_{};
};

/// @brief Abstract calendar deciding which dates are business days.
class Calendar {
 public:
  /// @brief Default-constructs a calendar.
  Calendar() = default;
  /// @brief Copy-constructs a calendar.
  Calendar(const Calendar&) = default;
  /// @brief Move-constructs a calendar.
  Calendar(Calendar&&) = default;
  /// @brief Copy-assigns a calendar.
  auto operator=(const Calendar&) -> Calendar& = default;
  /// @brief Move-assigns a calendar.
  auto operator=(Calendar&&) -> Calendar& = default;
  /// @brief Destroys the calendar.
  virtual ~Calendar() = default;

  /// @brief Returns whether @p date is a business day.
  ///
  /// @param date  Date to test.
  /// @return      @c true if @p date is a business day.
  [[nodiscard]] virtual auto is_business_day(const Date& date) const -> bool = 0;
};

/// @brief Calendar treating Saturday and Sunday as non-business days.
class WeekendCalendar : public Calendar {
 public:
  /// @brief Returns whether @p date falls on a weekday (Monday through Friday).
  ///
  /// @param date  Date to test.
  /// @return      @c true if @p date is not a Saturday or Sunday.
  [[nodiscard]] auto is_business_day(const Date& date) const -> bool override;
};

/// @brief Adjusts @p date onto a business day according to @p convention.
///
/// @param date        Date to adjust.
/// @param convention  Business day convention to apply.
/// @param calendar    Calendar defining business days.
/// @return            The adjusted business day (or @p date if already valid).
[[nodiscard]] auto adjust(const Date& date, BusinessDayConvention convention,
                          const Calendar& calendar) -> Date;

/// @brief Advances @p date by @p count business days.
///
/// @param date      Starting date.
/// @param count     Number of business days to move; may be negative.
/// @param calendar  Calendar defining business days.
/// @return          The resulting business day.
[[nodiscard]] auto add_business_days(const Date& date, int count, const Calendar& calendar) -> Date;

/// @brief Counts business days in the half-open interval between @p start and @p end.
///
/// Includes @p start when it is a business day and excludes @p end. The result is
/// negative when @p end precedes @p start.
///
/// @param start     Interval start date.
/// @param end       Interval end date.
/// @param calendar  Calendar defining business days.
/// @return          Signed count of business days between the two dates.
[[nodiscard]] auto business_days_between(const Date& start, const Date& end,
                                         const Calendar& calendar) -> int;

}  // namespace jaql::core
