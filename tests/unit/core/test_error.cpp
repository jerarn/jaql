#include <jaql/core/error.hpp>

#include <gtest/gtest.h>

#include <string>

using jaql::core::Code;
using jaql::core::Error;
using jaql::core::to_string;

// ---- to_string ----------------------------------------------------------------

TEST(Error, ToString_EveryCode_ReturnsNonEmptyLabel) {
  EXPECT_EQ(to_string(Code::Ok), "Ok");
  EXPECT_EQ(to_string(Code::InvalidArgument), "InvalidArgument");
  EXPECT_EQ(to_string(Code::OutOfRange), "OutOfRange");
  EXPECT_EQ(to_string(Code::InvalidDate), "InvalidDate");
  EXPECT_EQ(to_string(Code::NumericalFailure), "NumericalFailure");
  EXPECT_EQ(to_string(Code::NotFound), "NotFound");
  EXPECT_EQ(to_string(Code::ParseError), "ParseError");
  EXPECT_EQ(to_string(Code::InternalError), "InternalError");
}

// ---- what() -------------------------------------------------------------------

TEST(Error, What_ContainsCodeMessageFileNameAndLine) {
  constexpr std::string_view message = "bad input";
  const Error error{Code::ParseError, std::string{message}};

  const std::string description = error.what();

  EXPECT_NE(description.find("ParseError"), std::string::npos);
  EXPECT_NE(description.find(message), std::string::npos);
  EXPECT_NE(description.find("test_error.cpp"), std::string::npos);
  EXPECT_NE(description.find(':'), std::string::npos);
}

TEST(Error, SourceLocation_CapturedAtCallSiteNotInsideConstructor) {
  constexpr int expected_line = __LINE__ + 1;
  const Error error{Code::InvalidArgument, "callsite message"};

  EXPECT_FALSE(std::string{error.location().file_name()}.empty());
  EXPECT_EQ(error.location().line(), expected_line);
}
