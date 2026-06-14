#pragma once

#include <compare>
#include <concepts>
#include <cstddef>
#include <format>
#include <functional>
#include <type_traits>
#include <utility>

namespace jaql::core {

template <typename Tag, typename T, template <typename> class... Policies>
class StrongType;

namespace detail {

template <template <typename> class A, template <typename> class B>
inline constexpr bool is_same_policy_v = false;

template <template <typename> class Policy>
inline constexpr bool is_same_policy_v<Policy, Policy> = true;

/// @brief Detects whether @c Policy is listed among @c StrongType mix-ins.
template <template <typename> class Policy, typename Tag, typename T,
          template <typename> class... PolicyList>
inline constexpr bool strong_type_has_policy_v =
    sizeof...(PolicyList) != 0 && (is_same_policy_v<PolicyList, Policy> || ...);

template <template <typename> class Policy, typename Tag, typename T,
          template <typename> class... PolicyList>
consteval auto strong_type_has_policy() -> bool {
  return strong_type_has_policy_v<Policy, Tag, T, PolicyList...>;
}

}  // namespace detail

/// @brief CRTP policy enabling binary @c +, @c -, @c *, and @c / between two instances.
/// @tparam Derived  The strong type inheriting this policy.
template <typename Derived>
class Arithmetic {
 public:
  /// @brief Adds two strong-type values.
  [[nodiscard]] friend constexpr auto operator+(Derived lhs, Derived rhs) noexcept -> Derived {
    return Derived{lhs.value() + rhs.value()};
  }

  /// @brief Subtracts one strong-type value from another.
  [[nodiscard]] friend constexpr auto operator-(Derived lhs, Derived rhs) noexcept -> Derived {
    return Derived{lhs.value() - rhs.value()};
  }

  /// @brief Multiplies two strong-type values.
  [[nodiscard]] friend constexpr auto operator*(Derived lhs, Derived rhs) noexcept -> Derived {
    return Derived{lhs.value() * rhs.value()};
  }

  /// @brief Divides one strong-type value by another.
  [[nodiscard]] friend constexpr auto operator/(Derived lhs, Derived rhs) noexcept -> Derived {
    return Derived{lhs.value() / rhs.value()};
  }
};

/// @brief CRTP policy enabling three-way comparison and equality.
/// @tparam Derived  The strong type inheriting this policy.
template <typename Derived>
class Comparable {
 public:
  /// @brief Compares two strong-type values.
  [[nodiscard]] friend constexpr auto operator<=>(const Derived& lhs, const Derived& rhs) noexcept {
    return lhs.value() <=> rhs.value();
  }

  /// @brief Tests two strong-type values for equality.
  [[nodiscard]] friend constexpr auto operator==(const Derived& lhs, const Derived& rhs) noexcept
      -> bool {
    return lhs.value() == rhs.value();
  }
};

/// @brief CRTP marker policy enabling @c std::hash specialisation for container keys.
/// @tparam Derived  The strong type inheriting this policy.
template <typename Derived>
class Hashable {};

/// @brief CRTP marker policy enabling @c std::formatter specialisation for @c std::format.
/// @tparam Derived  The strong type inheriting this policy.
template <typename Derived>
class Formattable {};

/// @brief CRTP policy enabling scalar multiplication and division.
/// @tparam Derived  The strong type inheriting this policy.
template <typename Derived>
class Scalable {
 public:
  /// @brief Scales a strong-type value by a scalar.
  template <typename U>
    requires std::same_as<U, Derived>
  [[nodiscard]] friend constexpr auto operator*(U lhs, typename U::value_type scalar) noexcept
      -> U {
    return U{lhs.value() * scalar};
  }

  /// @brief Scales a strong-type value by a scalar (commutative overload).
  template <typename U>
    requires std::same_as<U, Derived>
  [[nodiscard]] friend constexpr auto operator*(typename U::value_type scalar, U rhs) noexcept
      -> U {
    return rhs * scalar;
  }

  /// @brief Divides a strong-type value by a scalar.
  template <typename U>
    requires std::same_as<U, Derived>
  [[nodiscard]] friend constexpr auto operator/(U lhs, typename U::value_type scalar) noexcept
      -> U {
    return U{lhs.value() / scalar};
  }
};

/// @brief CRTP policy enabling prefix and postfix increment and decrement.
/// @tparam Derived  The strong type inheriting this policy.
template <typename Derived>
class Incrementable {
 public:
  /// @brief Pre-increments the wrapped value.
  template <typename U>
    requires std::same_as<U, Derived>
  friend constexpr auto operator++(U& self) noexcept -> U& {
    ++self.value();
    return self;
  }

  /// @brief Post-increments the wrapped value.
  template <typename U>
    requires std::same_as<U, Derived>
  friend constexpr auto operator++(U& self, int) noexcept -> U {
    U copy{self};
    ++self;
    return copy;
  }

  /// @brief Pre-decrements the wrapped value.
  template <typename U>
    requires std::same_as<U, Derived>
  friend constexpr auto operator--(U& self) noexcept -> U& {
    --self.value();
    return self;
  }

  /// @brief Post-decrements the wrapped value.
  template <typename U>
    requires std::same_as<U, Derived>
  friend constexpr auto operator--(U& self, int) noexcept -> U {
    U copy{self};
    --self;
    return copy;
  }
};

/// @brief A zero-overhead tagged wrapper preventing implicit conversion between domain types.
///
/// Opt-in behaviour is supplied via CRTP policy mix-ins such as @c Arithmetic,
/// @c Comparable, @c Hashable, @c Formattable, @c Scalable, and @c Incrementable.
///
/// @tparam Tag       Empty tag type distinguishing domain quantities at compile time.
/// @tparam T         Underlying storage type.
/// @tparam Policies  Optional CRTP policy mix-ins controlling available operations.
template <typename Tag, typename T, template <typename> class... Policies>
class StrongType : public Policies<StrongType<Tag, T, Policies...>>... {
 public:
  using value_type = T;

  StrongType() = default;

  /// @brief Constructs a strong type from an explicit underlying value.
  /// @param value  Wrapped storage value.
  explicit constexpr StrongType(T value) noexcept(std::is_nothrow_move_constructible_v<T>)
      : value_(std::move(value)) {}

  /// @brief Returns read-only access to the wrapped value.
  [[nodiscard]] constexpr auto value() const& noexcept -> const T& { return value_; }

  /// @brief Returns mutable access to the wrapped value.
  [[nodiscard]] constexpr auto value() & noexcept -> T& { return value_; }

 private:
  T value_{};
};

static_assert(std::is_trivially_copyable_v<StrongType<struct SizeTag, double>>);
static_assert(sizeof(StrongType<struct SizeTag, double>) == sizeof(double));
static_assert(alignof(StrongType<struct SizeTag, double>) == alignof(double));

}  // namespace jaql::core

namespace std {

template <typename Tag, typename T, template <typename> class... Policies>
  requires(jaql::core::detail::strong_type_has_policy<jaql::core::Hashable, Tag, T, Policies...>())
struct hash<jaql::core::StrongType<Tag, T, Policies...>> {
  /// @brief Hashes the underlying value of a @c Hashable strong type.
  [[nodiscard]] auto operator()(
      const jaql::core::StrongType<Tag, T, Policies...>& value) const noexcept -> size_t {
    return hash<T>{}(value.value());
  }
};

template <typename Tag, typename T, template <typename> class... Policies>
  requires(
      jaql::core::detail::strong_type_has_policy_v<jaql::core::Formattable, Tag, T, Policies...>)
struct formatter<jaql::core::StrongType<Tag, T, Policies...>, char> {
  formatter<T, char> underlying_{};

  constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator {
    return underlying_.parse(ctx);
  }

  template <typename FormatContext>
  auto format(const jaql::core::StrongType<Tag, T, Policies...>& value, FormatContext& ctx) const ->
      typename FormatContext::iterator {
    return underlying_.format(value.value(), ctx);
  }
};

}  // namespace std
