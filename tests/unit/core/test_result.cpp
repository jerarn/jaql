#include <jaql/core/result.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

using jaql::core::all_of;
using jaql::core::Code;
using jaql::core::err;
using jaql::core::Error;
using jaql::core::ok;
using jaql::core::Result;
using jaql::core::result_cast;
using jaql::core::try_or;

// ---- ok() ---------------------------------------------------------------------

TEST(Result, OkVoid_ReturnsSuccess) {
  const Result<void> result = ok();
  EXPECT_TRUE(result.has_value());
}

TEST(Result, OkValue_ReturnsSuccessWithValue) {
  const Result<int> result = ok(42);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 42);
}

// ---- err() --------------------------------------------------------------------

TEST(Result, Err_ReturnsErrorWithCodeAndMessage) {
  constexpr std::string_view message = "invalid value";
  const Result<int> result = err<Code::InvalidArgument, int>(std::string{message});

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().code(), Code::InvalidArgument);
  EXPECT_EQ(result.error().message(), message);
}

// ---- result_cast() ------------------------------------------------------------

TEST(Result, ResultCast_Success_CastsValue) {
  const Result<int> input = ok(21);
  const Result<double> output = result_cast<double>(input);

  ASSERT_TRUE(output.has_value());
  EXPECT_DOUBLE_EQ(*output, 21.0);
}

TEST(Result, ResultCast_Error_PropagatesError) {
  constexpr std::string_view message = "not found";
  const Result<int> input = err<Code::NotFound, int>(std::string{message});
  const Result<double> output = result_cast<double>(input);

  ASSERT_FALSE(output.has_value());
  EXPECT_EQ(output.error().code(), Code::NotFound);
  EXPECT_EQ(output.error().message(), message);
}

// ---- try_or() -----------------------------------------------------------------

TEST(Result, TryOr_Success_ReturnsValue) {
  const Result<int> result = ok(7);
  EXPECT_EQ(try_or(result, 0), 7);
}

TEST(Result, TryOr_Error_ReturnsFallback) {
  const Result<int> result = err<Code::InternalError, int>("failure");
  EXPECT_EQ(try_or(result, 99), 99);
}

// ---- all_of() -----------------------------------------------------------------

TEST(Result, AllOf_EmptyInput_ReturnsEmptyVector) {
  const Result<std::vector<int>> result = all_of(std::vector<Result<int>>{});

  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->empty());
}

TEST(Result, AllOf_AllOk_ReturnsAllValues) {
  std::vector<Result<int>> inputs{ok(1), ok(2), ok(3)};
  const Result<std::vector<int>> result = all_of(std::move(inputs));

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, (std::vector<int>{1, 2, 3}));
}

TEST(Result, AllOf_FirstElementFailure_ReturnsFirstError) {
  constexpr std::string_view first_message = "first failure";
  constexpr std::string_view second_message = "second failure";

  std::vector<Result<int>> inputs{
      err<Code::InvalidArgument, int>(std::string{first_message}),
      err<Code::OutOfRange, int>(std::string{second_message}),
      ok(3),
  };
  const Result<std::vector<int>> result = all_of(std::move(inputs));

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().code(), Code::InvalidArgument);
  EXPECT_EQ(result.error().message(), first_message);
}

TEST(Result, AllOf_LastElementFailure_ReturnsLastError) {
  constexpr std::string_view message = "last failure";

  std::vector<Result<int>> inputs{ok(1), ok(2),
                                  err<Code::NumericalFailure, int>(std::string{message})};
  const Result<std::vector<int>> result = all_of(std::move(inputs));

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().code(), Code::NumericalFailure);
  EXPECT_EQ(result.error().message(), message);
}
