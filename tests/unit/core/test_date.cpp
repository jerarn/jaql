#include <jaql/core/date.hpp>

#include <jaql/core/error.hpp>
#include <jaql/core/result.hpp>

#include <gtest/gtest.h>

using jaql::core::add_business_days;
using jaql::core::adjust;
using jaql::core::business_days_between;
using jaql::core::BusinessDayConvention;
using jaql::core::Code;
using jaql::core::Date;
using jaql::core::DayOfWeek;
using jaql::core::days_in_month;
using jaql::core::is_leap_year;
using jaql::core::WeekendCalendar;

namespace {

/// @brief Builds a date from y/m/d, failing the test if the date is invalid.
auto make_date(int year, int month, int day) -> Date {
  auto result = Date::from_ymd(year, month, day);
  EXPECT_TRUE(result.has_value()) << "invalid test date " << year << '-' << month << '-' << day;
  return result.value_or(Date::from_serial(0));
}

}  // namespace

// ---- is_leap_year -------------------------------------------------------------

TEST(Date, IsLeapYear_KnownYears) {
  EXPECT_TRUE(is_leap_year(2000));
  EXPECT_FALSE(is_leap_year(1900));
  EXPECT_TRUE(is_leap_year(2024));
  EXPECT_FALSE(is_leap_year(2023));
}

// ---- days_in_month ------------------------------------------------------------

TEST(Date, DaysInMonth_EveryMonth) {
  EXPECT_EQ(days_in_month(1, 2023), 31);
  EXPECT_EQ(days_in_month(2, 2023), 28);
  EXPECT_EQ(days_in_month(2, 2024), 29);
  EXPECT_EQ(days_in_month(3, 2023), 31);
  EXPECT_EQ(days_in_month(4, 2023), 30);
  EXPECT_EQ(days_in_month(5, 2023), 31);
  EXPECT_EQ(days_in_month(6, 2023), 30);
  EXPECT_EQ(days_in_month(7, 2023), 31);
  EXPECT_EQ(days_in_month(8, 2023), 31);
  EXPECT_EQ(days_in_month(9, 2023), 30);
  EXPECT_EQ(days_in_month(10, 2023), 31);
  EXPECT_EQ(days_in_month(11, 2023), 30);
  EXPECT_EQ(days_in_month(12, 2023), 31);
}

// ---- from_ymd -----------------------------------------------------------------

TEST(Date, FromYmd_ValidInput_RoundTrips) {
  const Date date = make_date(2024, 2, 29);
  EXPECT_EQ(date.year(), 2024);
  EXPECT_EQ(date.month(), 2);
  EXPECT_EQ(date.day(), 29);
}

TEST(Date, FromYmd_InvalidMonth_ReturnsError) {
  const auto result = Date::from_ymd(2024, 13, 1);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().code(), Code::InvalidDate);
}

TEST(Date, FromYmd_InvalidDay_ReturnsError) {
  const auto result = Date::from_ymd(2024, 1, 32);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().code(), Code::InvalidDate);
}

TEST(Date, FromYmd_February30_ReturnsError) {
  const auto result = Date::from_ymd(2024, 2, 30);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().code(), Code::InvalidDate);
}

// ---- add_days -----------------------------------------------------------------

TEST(Date, AddDays_Positive) {
  EXPECT_EQ(make_date(2024, 1, 15).add_days(10), make_date(2024, 1, 25));
}

TEST(Date, AddDays_Negative) {
  EXPECT_EQ(make_date(2024, 1, 15).add_days(-20), make_date(2023, 12, 26));
}

TEST(Date, AddDays_CrossingMonthBoundary) {
  EXPECT_EQ(make_date(2024, 1, 25).add_days(10), make_date(2024, 2, 4));
}

TEST(Date, AddDays_CrossingYearBoundary) {
  EXPECT_EQ(make_date(2023, 12, 28).add_days(5), make_date(2024, 1, 2));
}

// ---- add_months ---------------------------------------------------------------

TEST(Date, AddMonths_Normal) {
  EXPECT_EQ(make_date(2024, 1, 15).add_months(1), make_date(2024, 2, 15));
}

TEST(Date, AddMonths_EndOfMonthClampLeap) {
  EXPECT_EQ(make_date(2024, 1, 31).add_months(1), make_date(2024, 2, 29));
}

TEST(Date, AddMonths_EndOfMonthClampNonLeap) {
  EXPECT_EQ(make_date(2023, 1, 31).add_months(1), make_date(2023, 2, 28));
}

TEST(Date, AddMonths_Negative) {
  EXPECT_EQ(make_date(2024, 3, 31).add_months(-1), make_date(2024, 2, 29));
}

// ---- add_years ----------------------------------------------------------------

TEST(Date, AddYears_LeapDaySourceClampsToNonLeapTarget) {
  EXPECT_EQ(make_date(2024, 2, 29).add_years(1), make_date(2025, 2, 28));
}

TEST(Date, AddYears_Normal) {
  EXPECT_EQ(make_date(2024, 6, 15).add_years(2), make_date(2026, 6, 15));
}

// ---- day_of_week --------------------------------------------------------------

TEST(Date, DayOfWeek_EpochIsThursday) {
  EXPECT_EQ(Date::from_serial(0).day_of_week(), DayOfWeek::Thursday);
}

TEST(Date, DayOfWeek_KnownWeek) {
  EXPECT_EQ(make_date(2024, 1, 1).day_of_week(), DayOfWeek::Monday);
  EXPECT_EQ(make_date(2024, 1, 2).day_of_week(), DayOfWeek::Tuesday);
  EXPECT_EQ(make_date(2024, 1, 3).day_of_week(), DayOfWeek::Wednesday);
  EXPECT_EQ(make_date(2024, 1, 4).day_of_week(), DayOfWeek::Thursday);
  EXPECT_EQ(make_date(2024, 1, 5).day_of_week(), DayOfWeek::Friday);
  EXPECT_EQ(make_date(2024, 1, 6).day_of_week(), DayOfWeek::Saturday);
  EXPECT_EQ(make_date(2024, 1, 7).day_of_week(), DayOfWeek::Sunday);
}

// ---- WeekendCalendar ----------------------------------------------------------

TEST(Date, WeekendCalendar_IsBusinessDay) {
  const WeekendCalendar calendar;
  EXPECT_TRUE(calendar.is_business_day(make_date(2024, 1, 1)));   // Monday
  EXPECT_TRUE(calendar.is_business_day(make_date(2024, 1, 2)));   // Tuesday
  EXPECT_TRUE(calendar.is_business_day(make_date(2024, 1, 3)));   // Wednesday
  EXPECT_TRUE(calendar.is_business_day(make_date(2024, 1, 4)));   // Thursday
  EXPECT_TRUE(calendar.is_business_day(make_date(2024, 1, 5)));   // Friday
  EXPECT_FALSE(calendar.is_business_day(make_date(2024, 1, 6)));  // Saturday
  EXPECT_FALSE(calendar.is_business_day(make_date(2024, 1, 7)));  // Sunday
}

// ---- adjust -------------------------------------------------------------------

TEST(Date, Adjust_Unadjusted_ReturnsInput) {
  const WeekendCalendar calendar;
  const Date saturday = make_date(2024, 3, 30);
  EXPECT_EQ(adjust(saturday, BusinessDayConvention::Unadjusted, calendar), saturday);
}

TEST(Date, Adjust_Following) {
  const WeekendCalendar calendar;
  // Saturday 2024-03-30 rolls forward into April.
  EXPECT_EQ(adjust(make_date(2024, 3, 30), BusinessDayConvention::Following, calendar),
            make_date(2024, 4, 1));
}

TEST(Date, Adjust_ModifiedFollowing_RollsBackAtMonthEnd) {
  const WeekendCalendar calendar;
  // Following would cross into April, so Modified rolls back to Friday 2024-03-29.
  EXPECT_EQ(adjust(make_date(2024, 3, 30), BusinessDayConvention::ModifiedFollowing, calendar),
            make_date(2024, 3, 29));
}

TEST(Date, Adjust_Preceding) {
  const WeekendCalendar calendar;
  EXPECT_EQ(adjust(make_date(2024, 3, 30), BusinessDayConvention::Preceding, calendar),
            make_date(2024, 3, 29));
}

TEST(Date, Adjust_ModifiedPreceding_RollsForwardAtMonthStart) {
  const WeekendCalendar calendar;
  // Preceding would cross into May, so Modified rolls forward to Monday 2024-06-03.
  EXPECT_EQ(adjust(make_date(2024, 6, 1), BusinessDayConvention::ModifiedPreceding, calendar),
            make_date(2024, 6, 3));
}

// ---- add_business_days --------------------------------------------------------

TEST(Date, AddBusinessDays_PositiveCrossingMultipleWeekends) {
  const WeekendCalendar calendar;
  // From Friday 2024-03-29, six business days cross two weekends to Monday 2024-04-08.
  EXPECT_EQ(add_business_days(make_date(2024, 3, 29), 6, calendar), make_date(2024, 4, 8));
}

TEST(Date, AddBusinessDays_NegativeCrossingMultipleWeekends) {
  const WeekendCalendar calendar;
  EXPECT_EQ(add_business_days(make_date(2024, 4, 8), -6, calendar), make_date(2024, 3, 29));
}

TEST(Date, AddBusinessDays_ZeroIsIdentity) {
  const WeekendCalendar calendar;
  const Date date = make_date(2024, 3, 29);
  EXPECT_EQ(add_business_days(date, 0, calendar), date);
}

// ---- business_days_between ----------------------------------------------------

TEST(Date, BusinessDaysBetween_WithinWeek) {
  const WeekendCalendar calendar;
  // [Mon 2024-01-01, Fri 2024-01-05): Mon, Tue, Wed, Thu = 4.
  EXPECT_EQ(business_days_between(make_date(2024, 1, 1), make_date(2024, 1, 5), calendar), 4);
}

TEST(Date, BusinessDaysBetween_AcrossWeekends) {
  const WeekendCalendar calendar;
  EXPECT_EQ(business_days_between(make_date(2024, 3, 29), make_date(2024, 4, 8), calendar), 6);
}

TEST(Date, BusinessDaysBetween_ReversedIsNegative) {
  const WeekendCalendar calendar;
  EXPECT_EQ(business_days_between(make_date(2024, 4, 8), make_date(2024, 3, 29), calendar), -6);
}

TEST(Date, BusinessDaysBetween_SameDateIsZero) {
  const WeekendCalendar calendar;
  const Date date = make_date(2024, 1, 1);
  EXPECT_EQ(business_days_between(date, date, calendar), 0);
}

// ---- comparison and difference ------------------------------------------------

TEST(Date, ThreeWayComparison) {
  const Date earlier = make_date(2024, 1, 1);
  const Date later = make_date(2024, 1, 2);
  EXPECT_TRUE(earlier < later);
  EXPECT_TRUE(later > earlier);
  EXPECT_TRUE(earlier == make_date(2024, 1, 1));
  EXPECT_TRUE(earlier <= make_date(2024, 1, 1));
  EXPECT_TRUE(earlier >= make_date(2024, 1, 1));
}

TEST(Date, Difference_PositiveAndNegative) {
  const Date start = make_date(2024, 1, 1);
  const Date end = make_date(2024, 1, 10);
  EXPECT_EQ(end - start, 9);
  EXPECT_EQ(start - end, -9);
}
