#pragma once

#include <source_location>
#include <string>
#include <string_view>

namespace jaql::core {

/// @brief Domain error classification codes used throughout JAQL.
enum class Code {
  /// @brief Operation succeeded.
  Ok,
  /// @brief One or more arguments are invalid.
  InvalidArgument,
  /// @brief A value lies outside its permitted range.
  OutOfRange,
  /// @brief A calendar date or date operation is invalid.
  InvalidDate,
  /// @brief A numerical computation failed or produced an invalid result.
  NumericalFailure,
  /// @brief A requested resource was not found.
  NotFound,
  /// @brief Input could not be parsed.
  ParseError,
  /// @brief An unexpected internal failure occurred.
  InternalError,
};

/// @brief Returns a stable string representation of @p code.
///
/// @param code  Error classification code.
/// @return      Human-readable label for @p code.
[[nodiscard]] auto to_string(Code code) -> std::string_view;

/// @brief Structured error value propagated through @c Result\<T\>.
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
  /// @return Error classification code.
  [[nodiscard]] auto code() const noexcept -> Code { return code_; }

  /// @brief Returns the error message.
  /// @return Human-readable description of the failure.
  [[nodiscard]] auto message() const noexcept -> const std::string& { return message_; }

  /// @brief Returns the source location captured at the construction call site.
  /// @return Source location recorded at construction.
  [[nodiscard]] auto location() const noexcept -> const std::source_location& { return location_; }

  /// @brief Returns a formatted string combining code, message, and source location.
  /// @return Diagnostic string suitable for logging or display.
  [[nodiscard]] auto what() const -> std::string;

 private:
  Code code_;
  std::string message_;
  std::source_location location_;
};

}  // namespace jaql::core
