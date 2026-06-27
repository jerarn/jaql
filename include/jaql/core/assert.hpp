#pragma once

#include <jaql/infra/logger.hpp>

#include <cstdlib>
#include <exception>
#include <format>
#include <source_location>
#include <string>
#include <string_view>

namespace jaql::core {

/// @brief Context passed to a violation handler when an assertion macro fails.
struct ViolationInfo {
  /// @brief Stringified expression that evaluated to false.
  std::string_view expression;
  /// @brief Optional caller-supplied diagnostic message.
  std::string_view message;
  /// @brief Source location of the failing check.
  std::source_location location{};
};

/// @brief Callback invoked when @c JAQL_ASSERT, @c JAQL_EXPECTS, or @c JAQL_ENSURES fails.
using ViolationHandler = void (*)(const ViolationInfo& info);

/// @brief Exception type thrown by @c throwing_handler for test isolation.
class AssertionViolation : public std::runtime_error {
 public:
  /// @brief Constructs an exception describing @p info.
  /// @param info  Violation context captured at the failing macro site.
  explicit AssertionViolation(const ViolationInfo& info);

  /// @brief Returns the violation context associated with this exception.
  /// @return Reference to the stored violation context.
  [[nodiscard]] auto info() const noexcept -> const ViolationInfo& { return info_; }

 private:
  ViolationInfo info_;
};

/// @brief Default handler that terminates the process without throwing.
/// @param info  Violation context captured at the failing macro site.
[[noreturn]] inline void abort_handler(const ViolationInfo& info) {
  static_cast<void>(info);
  std::abort();
}

namespace detail {

/// @brief Inline-static storage for the active violation handler.
/// @return Reference to the stored violation handler.
[[nodiscard]] inline auto violation_handler_storage() -> ViolationHandler& {
  static ViolationHandler handler = abort_handler;
  return handler;
}

/// @brief Formats @p info into a single human-readable diagnostic string.
/// @param info  Violation context to format.
/// @return      Diagnostic string including file, line, and expression.
[[nodiscard]] inline auto format_violation(const ViolationInfo& info) -> std::string {
  if (info.message.empty()) {
    return std::format("{}:{}: {}", info.location.file_name(), info.location.line(),
                       info.expression);
  }
  return std::format("{}:{}: {} — {}", info.location.file_name(), info.location.line(),
                     info.expression, info.message);
}

}  // namespace detail

/// @brief Returns the currently installed violation handler.
/// @return Active callback invoked on assertion failure.
[[nodiscard]] inline auto get_violation_handler() -> ViolationHandler {
  return detail::violation_handler_storage();
}

/// @brief Installs @p handler as the active violation handler.
/// @param handler  Callback invoked on assertion failure; must not be null.
inline void set_violation_handler(ViolationHandler handler) {
  detail::violation_handler_storage() = handler;
}

namespace detail {

/// @brief Reports a failed assertion to the active violation handler.
/// @param expression  Stringified expression that evaluated to false.
/// @param message     Optional caller-supplied diagnostic message.
/// @param location    Source location of the failing check.
inline void report_violation(std::string_view expression, std::string_view message = {},
                             std::source_location location = std::source_location::current()) {
  const ViolationInfo info{expression, message, location};
  get_violation_handler()(info);
}

}  // namespace detail

inline AssertionViolation::AssertionViolation(const ViolationInfo& info)
    : std::runtime_error(detail::format_violation(info)), info_(info) {}

/// @brief Handler that throws @c AssertionViolation for test isolation.
/// @param info  Violation context captured at the failing macro site.
inline void throwing_handler(const ViolationInfo& info) { throw AssertionViolation{info}; }

/// @brief Handler that logs the violation via @c infra::get_logger and then aborts.
/// @param info  Violation context captured at the failing macro site.
[[noreturn]] inline void log_handler(const ViolationInfo& info) {
  infra::get_logger("jaql.assert")->critical("{}", detail::format_violation(info));
  std::abort();
}

}  // namespace jaql::core

#if !defined(NDEBUG)
#define JAQL_DETAIL_VIOLATION_CHECK(condition, ...) \
  ((condition) ? static_cast<void>(0)               \
               : ::jaql::core::detail::report_violation(#condition __VA_OPT__(, ) __VA_ARGS__))
#else
#define JAQL_DETAIL_VIOLATION_CHECK(condition, ...) static_cast<void>(0)
#endif

#if !defined(NDEBUG) || defined(JAQL_ENABLE_CONTRACTS)
#define JAQL_DETAIL_CONTRACT_CHECK(condition, ...) \
  ((condition) ? static_cast<void>(0)              \
               : ::jaql::core::detail::report_violation(#condition __VA_OPT__(, ) __VA_ARGS__))
#else
#define JAQL_DETAIL_CONTRACT_CHECK(condition, ...) static_cast<void>(0)
#endif

/// @brief Debug assertion stripped when @c NDEBUG is defined.
#define JAQL_ASSERT(condition, ...) \
  JAQL_DETAIL_VIOLATION_CHECK(condition __VA_OPT__(, ) __VA_ARGS__)

/// @brief Precondition check; remains enabled when @c JAQL_ENABLE_CONTRACTS is defined.
#define JAQL_EXPECTS(condition, ...) \
  JAQL_DETAIL_CONTRACT_CHECK(condition __VA_OPT__(, ) __VA_ARGS__)

/// @brief Postcondition check; remains enabled when @c JAQL_ENABLE_CONTRACTS is defined.
#define JAQL_ENSURES(condition, ...) \
  JAQL_DETAIL_CONTRACT_CHECK(condition __VA_OPT__(, ) __VA_ARGS__)
