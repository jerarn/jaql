#pragma once

#include <source_location>
#include <string>
#include <string_view>

namespace jaql::core {

/// @brief Domain error classification codes used throughout JAQL.
enum class Code {
  Ok,
  InvalidArgument,
  OutOfRange,
  InvalidDate,
  NumericalFailure,
  NotFound,
  ParseError,
  InternalError,
};

/// @brief Returns a stable string representation of @p code.
///
/// @param code  Error classification code.
/// @return      Human-readable label for @p code.
[[nodiscard]] auto to_string(Code code) -> std::string_view;

/// @brief Structured error value propagated through @c Result<T>.
///
/// Captures a classification code, a caller-supplied message, and the source
/// location at the construction call site.
class Error {
 public:
  /// @brief Constructs an error with the given code, message, and source location.
  ///
  /// @param code      Error classification code.
  /// @param message   Human-readable description of the failure.
  /// @param location  Call-site source location; defaults to the caller's location.
  Error(Code code, std::string message,
        std::source_location location = std::source_location::current());

  /// @brief Returns the error classification code.
  [[nodiscard]] auto code() const noexcept -> Code { return code_; }

  /// @brief Returns the error message.
  [[nodiscard]] auto message() const noexcept -> const std::string& { return message_; }

  /// @brief Returns the source location captured at the construction call site.
  [[nodiscard]] auto location() const noexcept -> const std::source_location& { return location_; }

  /// @brief Returns a formatted string combining code, message, and source location.
  [[nodiscard]] auto what() const -> std::string;

 private:
  Code code_;
  std::string message_;
  std::source_location location_;
};

}  // namespace jaql::core
