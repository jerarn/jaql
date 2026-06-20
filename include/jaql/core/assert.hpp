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
  std::string_view expression;
  std::string_view message;
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
  [[nodiscard]] auto info() const noexcept -> const ViolationInfo& { return info_; }

 private:
  ViolationInfo info_;
};

/// @brief Returns the currently installed violation handler.
[[nodiscard]] auto get_violation_handler() -> ViolationHandler;

/// @brief Installs @p handler as the active violation handler.
/// @param handler  Callback invoked on assertion failure; must not be null.
void set_violation_handler(ViolationHandler handler);

/// @brief Default handler that terminates the process without throwing.
[[noreturn]] void abort_handler(const ViolationInfo& info);

/// @brief Handler that throws @c AssertionViolation for test isolation.
void throwing_handler(const ViolationInfo& info);

/// @brief Handler that logs the violation via @c infra::get_logger and then aborts.
[[noreturn]] void log_handler(const ViolationInfo& info);

namespace detail {

/// @brief Formats @p info into a single human-readable diagnostic string.
[[nodiscard]] auto format_violation(const ViolationInfo& info) -> std::string;

/// @brief Inline-static storage for the active violation handler.
[[nodiscard]] inline auto violation_handler_storage() -> ViolationHandler& {
  static ViolationHandler handler = abort_handler;
  return handler;
}

/// @brief Reports a failed assertion to the active violation handler.
inline void report_violation(std::string_view expression, std::string_view message = {},
                             std::source_location location = std::source_location::current()) {
  const ViolationInfo info{expression, message, location};
  get_violation_handler()(info);
}

}  // namespace detail

inline auto get_violation_handler() -> ViolationHandler {
  return detail::violation_handler_storage();
}

inline void set_violation_handler(ViolationHandler handler) {
  detail::violation_handler_storage() = handler;
}

inline auto detail::format_violation(const ViolationInfo& info) -> std::string {
  if (info.message.empty()) {
    return std::format("{}:{}: {}", info.location.file_name(), info.location.line(),
                       info.expression);
  }
  return std::format("{}:{}: {} — {}", info.location.file_name(), info.location.line(),
                     info.expression, info.message);
}

inline AssertionViolation::AssertionViolation(const ViolationInfo& info)
    : std::runtime_error(detail::format_violation(info)), info_(info) {}

[[noreturn]] inline void abort_handler(const ViolationInfo& /*info*/) { std::abort(); }

inline void throwing_handler(const ViolationInfo& info) { throw AssertionViolation{info}; }

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
