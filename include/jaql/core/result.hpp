#pragma once

#include <jaql/core/error.hpp>

#include <tl/expected.hpp>

#include <utility>
#include <vector>

namespace jaql::core {

/// @brief Success-or-error result type used for recoverable failures.
///
/// @tparam T  Value type carried on the success path.
template <typename T>
using Result = tl::expected<T, Error>;

/// @brief Creates a successful void result.
///
/// @return  A successful @c Result\<void\>.
[[nodiscard]] inline auto ok() noexcept -> Result<void> { return {}; }

/// @brief Creates a successful result carrying @p value.
///
/// @tparam T     Value type of the result.
/// @param value  Success value to store.
/// @return       A successful @c Result\<T\> containing @p value.
template <typename T>
[[nodiscard]] constexpr auto ok(T value) noexcept -> Result<T> {
  return std::move(value);
}

/// @brief Creates an error result with the given code and message.
///
/// @tparam C         Error classification code.
/// @tparam T         Value type of the result (defaults to @c void).
/// @param message    Human-readable description of the failure.
/// @param location   Call-site source location; defaults to the caller's location.
/// @return           A failed @c Result\<T\> carrying the constructed @c Error.
template <Code C, typename T = void>
[[nodiscard]] auto err(std::string message,
                       std::source_location location = std::source_location::current())
    -> Result<T> {
  return tl::unexpected(Error{C, std::move(message), location});
}

/// @brief Casts the success value of a result to a new type, propagating errors.
///
/// @tparam U      Target value type.
/// @tparam T      Source value type.
/// @param result  Result to cast.
/// @return        A @c Result\<U\> with the converted value, or the original error.
template <typename U, typename T>
[[nodiscard]] auto result_cast(Result<T> result) -> Result<U> {
  if (!result) {
    return tl::unexpected(result.error());
  }
  return static_cast<U>(*result);
}

/// @brief Returns the success value, or @p fallback when the result holds an error.
///
/// @tparam T         Value type of the result.
/// @param result     Result to unwrap.
/// @param fallback   Value returned when @p result is an error.
/// @return           The success value, or @p fallback.
template <typename T>
[[nodiscard]] constexpr auto try_or(Result<T> result, T fallback) noexcept -> T {
  return result.value_or(std::move(fallback));
}

/// @brief Combines a sequence of results, failing fast on the first error.
///
/// @tparam T       Value type of each input result.
/// @param results  Sequence of results to combine.
/// @return         A vector of all success values, or the first error encountered.
template <typename T>
[[nodiscard]] auto all_of(std::vector<Result<T>> results) -> Result<std::vector<T>> {
  std::vector<T> values;
  values.reserve(results.size());
  for (auto& result : results) {
    if (!result) {
      return tl::unexpected(result.error());
    }
    values.push_back(std::move(*result));
  }
  return values;
}

}  // namespace jaql::core
