#include <jaql/core/strong_type.hpp>

#include <gtest/gtest.h>

#include <concepts>
#include <format>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace jaql::core::test {

struct RateTag {};
struct SpreadTag {};
struct CounterTag {};

using PlainRate = StrongType<RateTag, double>;
using Rate = StrongType<RateTag, double, Arithmetic, Comparable, Hashable, Formattable, Scalable,
                       Incrementable>;
using Spread = StrongType<SpreadTag, double, Comparable>;
using Counter = StrongType<CounterTag, int, Incrementable, Comparable>;

static_assert(!std::is_convertible_v<double, PlainRate>,
              "StrongType must not be implicitly constructible from underlying type");
static_assert(!std::is_constructible_v<Rate, Spread>,
              "Rate must not be constructible from Spread");
static_assert(sizeof(Rate) == sizeof(double));
static_assert(alignof(Rate) == alignof(double));

TEST(StrongType, ExplicitConstructionSucceeds) {
  const Rate rate{0.05};
  EXPECT_NEAR(rate.value(), 0.05, 1e-15);
}

TEST(StrongType, MoveSemantics_PreservesValue) {
  Rate original{0.05};
  Rate moved = std::move(original);
  EXPECT_NEAR(moved.value(), 0.05, 1e-15);
}

TEST(StrongType, Arithmetic_BinaryOperatorsCombineValues) {
  const Rate lhs{0.05};
  const Rate rhs{0.02};

  EXPECT_NEAR((lhs + rhs).value(), 0.07, 1e-15);
  EXPECT_NEAR((lhs - rhs).value(), 0.03, 1e-15);
  EXPECT_NEAR((lhs * rhs).value(), 0.001, 1e-15);
  EXPECT_NEAR((lhs / rhs).value(), 2.5, 1e-15);
}

TEST(StrongType, Comparable_ThreeWayComparisonOrdersValues) {
  const Rate lower{0.02};
  const Rate higher{0.05};

  EXPECT_LT(lower <=> higher, 0);
  EXPECT_GT(higher <=> lower, 0);
  EXPECT_EQ(lower <=> lower, 0);
  EXPECT_TRUE(lower == lower);
  EXPECT_FALSE(lower == higher);
}

TEST(StrongType, Scalable_ScalarMultiplicationAndDivision) {
  const Rate rate{0.05};

  EXPECT_NEAR((rate * 2.0).value(), 0.10, 1e-15);
  EXPECT_NEAR((2.0 * rate).value(), 0.10, 1e-15);
  EXPECT_NEAR((rate / 2.0).value(), 0.025, 1e-15);
}

TEST(StrongType, Hashable_EnablesUnorderedMapLookup) {
  std::unordered_map<Rate, int> rates{{Rate{0.05}, 1}, {Rate{0.10}, 2}};

  EXPECT_EQ(rates.at(Rate{0.05}), 1);
  EXPECT_EQ(rates.at(Rate{0.10}), 2);
}

TEST(StrongType, Formattable_FormatsUnderlyingValue) {
  const Rate rate{0.05};
  EXPECT_EQ(std::format("{}", rate), "0.05");
}

TEST(StrongType, Incrementable_PrefixAndPostfixOperators) {
  Counter counter{1};

  EXPECT_EQ((++counter).value(), 2);
  EXPECT_EQ(counter.value(), 2);

  const Counter before = counter++;
  EXPECT_EQ(before.value(), 2);
  EXPECT_EQ(counter.value(), 3);

  EXPECT_EQ((--counter).value(), 2);
  EXPECT_EQ(counter.value(), 2);

  const Counter after = counter--;
  EXPECT_EQ(after.value(), 2);
  EXPECT_EQ(counter.value(), 1);
}

TEST(StrongType, MissingArithmeticPolicy_OperatorPlusIsIllformed) {
  static_assert(!requires(Spread lhs, Spread rhs) { lhs + rhs; },
                "Spread without Arithmetic must not support operator+");
}

TEST(StrongType, MissingHashablePolicy_HashSpecialisationIsAbsent) {
  static_assert(!detail::strong_type_has_policy<Hashable, SpreadTag, double, Comparable>(),
                "Spread without Hashable must not enable std::hash");
}

TEST(StrongType, MissingFormattablePolicy_FormatIsIllformed) {
  static_assert(!requires(const Spread spread) { std::format("{}", spread); },
                "Spread without Formattable must not support std::format");
}

TEST(StrongType, MissingScalablePolicy_ScalarMultiplyIsIllformed) {
  static_assert(!requires(Spread spread) { spread * 2.0; },
                "Spread without Scalable must not support scalar multiplication");
}

TEST(StrongType, MissingIncrementablePolicy_IncrementIsIllformed) {
  static_assert(!requires(Spread spread) { ++spread; },
                "Spread without Incrementable must not support operator++");
}

}  // namespace jaql::core::test
