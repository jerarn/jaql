#pragma once

#include <compare>
#include <concepts>
#include <cstddef>
#include <format>
#include <functional>
#include <type_traits>
#include <utility>

namespace jaql::core {

namespace detail {

/// @brief Detects whether two policy templates refer to the same type.
template <template <typename> class A, template <typename> class B>
inline constexpr bool is_same_policy_v = false;

/// @brief Specialisation reporting that a policy matches itself.
template <template <typename> class Policy>
inline constexpr bool is_same_policy_v<Policy, Policy> = true;

/// @brief Detects whether @c Policy is listed among @c StrongType mix-ins.
template <template <typename> class Policy, typename Tag, typename T,
          template <typename> class... PolicyList>
inline constexpr bool strong_type_has_policy_v =
    sizeof...(PolicyList) != 0 && (is_same_policy_v<PolicyList, Policy> || ...);

/// @brief Constexpr helper returning @c strong_type_has_policy_v.
/// @tparam Policy      Policy template to test for.
/// @tparam Tag         Strong-type tag type.
/// @tparam T           Underlying storage type.
/// @tparam PolicyList  Policy mix-ins applied to the strong type.
/// @return             @c true when @c Policy appears in @c PolicyList.
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
  /// @param lhs  Left-hand operand.
  /// @param rhs  Right-hand operand.
  /// @return     Sum of the two wrapped values.
  [[nodiscard]] friend constexpr auto operator+(Derived lhs, Derived rhs) noexcept -> Derived {
    return Derived{lhs.value() + rhs.value()};
  }

  /// @brief Subtracts one strong-type value from another.
  /// @param lhs  Minuend.
  /// @param rhs  Subtrahend.
  /// @return     Difference of the two wrapped values.
  [[nodiscard]] friend constexpr auto operator-(Derived lhs, Derived rhs) noexcept -> Derived {
    return Derived{lhs.value() - rhs.value()};
  }

  /// @brief Multiplies two strong-type values.
  /// @param lhs  Left-hand operand.
  /// @param rhs  Right-hand operand.
  /// @return     Product of the two wrapped values.
  [[nodiscard]] friend constexpr auto operator*(Derived lhs, Derived rhs) noexcept -> Derived {
    return Derived{lhs.value() * rhs.value()};
  }

  /// @brief Divides one strong-type value by another.
  /// @param lhs  Dividend.
  /// @param rhs  Divisor.
  /// @return     Quotient of the two wrapped values.
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
  /// @param lhs  Left-hand operand.
  /// @param rhs  Right-hand operand.
  /// @return     Three-way comparison of the wrapped values.
  [[nodiscard]] friend constexpr auto operator<=>(const Derived& lhs, const Derived& rhs) noexcept {
    return lhs.value() <=> rhs.value();
  }

  /// @brief Tests two strong-type values for equality.
  /// @param lhs  Left-hand operand.
  /// @param rhs  Right-hand operand.
  /// @return     @c true if the wrapped values compare equal.
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
  /// @tparam U  Strong type inheriting @c Scalable.
  /// @param lhs     Value to scale.
  /// @param scalar  Multiplier applied to the wrapped value.
  /// @return        Scaled strong-type value.
  template <typename U>
    requires std::same_as<U, Derived>
  [[nodiscard]] friend constexpr auto operator*(U lhs, typename U::value_type scalar) noexcept
      -> U {
    return U{lhs.value() * scalar};
  }

  /// @brief Scales a strong-type value by a scalar (commutative overload).
  /// @tparam U  Strong type inheriting @c Scalable.
  /// @param scalar  Multiplier applied to the wrapped value.
  /// @param rhs     Value to scale.
  /// @return        Scaled strong-type value.
  template <typename U>
    requires std::same_as<U, Derived>
  [[nodiscard]] friend constexpr auto operator*(typename U::value_type scalar, U rhs) noexcept
      -> U {
    return rhs * scalar;
  }

  /// @brief Divides a strong-type value by a scalar.
  /// @tparam U  Strong type inheriting @c Scalable.
  /// @param lhs     Dividend.
  /// @param scalar  Divisor applied to the wrapped value.
  /// @return        Quotient as a strong-type value.
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
  /// @tparam U  Strong type inheriting @c Incrementable.
  /// @param self  Value to increment.
  /// @return      Reference to @p self after increment.
  template <typename U>
    requires std::same_as<U, Derived>
  friend constexpr auto operator++(U& self) noexcept -> U& {
    ++self.value();
    return self;
  }

  /// @brief Post-increments the wrapped value.
  /// @tparam U  Strong type inheriting @c Incrementable.
  /// @param self  Value to increment.
  /// @return      Copy of @p self before increment.
  template <typename U>
    requires std::same_as<U, Derived>
  friend constexpr auto operator++(U& self, int) noexcept -> U {
    U copy{self};
    ++self;
    return copy;
  }

  /// @brief Pre-decrements the wrapped value.
  /// @tparam U  Strong type inheriting @c Incrementable.
  /// @param self  Value to decrement.
  /// @return      Reference to @p self after decrement.
  template <typename U>
    requires std::same_as<U, Derived>
  friend constexpr auto operator--(U& self) noexcept -> U& {
    --self.value();
    return self;
  }

  /// @brief Post-decrements the wrapped value.
  /// @tparam U  Strong type inheriting @c Incrementable.
  /// @param self  Value to decrement.
  /// @return      Copy of @p self before decrement.
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
  /// @brief Underlying storage type wrapped by this strong type.
  using value_type = T;

  /// @brief Default-constructs the wrapped value.
  StrongType() = default;

  /// @brief Constructs a strong type from an explicit underlying value.
  /// @param value  Wrapped storage value.
  explicit constexpr StrongType(T value) noexcept(std::is_nothrow_move_constructible_v<T>)
      : value_(std::move(value)) {}

  /// @brief Returns read-only access to the wrapped value.
  /// @return Const reference to the underlying storage.
  [[nodiscard]] constexpr auto value() const& noexcept -> const T& { return value_; }

  /// @brief Returns mutable access to the wrapped value.
  /// @return Mutable reference to the underlying storage.
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
/// @brief @c std::hash specialisation for @c Hashable strong types.
struct hash<jaql::core::StrongType<Tag, T, Policies...>> {
  /// @brief Hashes the underlying value of a @c Hashable strong type.
  /// @param value  Strong type instance to hash.
  /// @return       Hash of the wrapped value.
  [[nodiscard]] auto operator()(
      const jaql::core::StrongType<Tag, T, Policies...>& value) const noexcept -> size_t {
    return hash<T>{}(value.value());
  }
};

template <typename Tag, typename T, template <typename> class... Policies>
  requires(
      jaql::core::detail::strong_type_has_policy_v<jaql::core::Formattable, Tag, T, Policies...>)
/// @brief @c std::formatter specialisation for @c Formattable strong types.
struct formatter<jaql::core::StrongType<Tag, T, Policies...>, char> {
  /// @brief Formatter for the underlying storage type.
  formatter<T, char> underlying{};

  /// @brief Parses the format specification from the format context.
  /// @param ctx  Format parse context.
  /// @return     Iterator past the parsed specification.
  constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator {
    return underlying.parse(ctx);
  }

  /// @brief Writes the formatted strong-type value to the output.
  /// @tparam FormatContext  Output format context type.
  /// @param value  Strong type instance to format.
  /// @param ctx    Format output context.
  /// @return       Iterator past the written output.
  template <typename FormatContext>
  auto format(const jaql::core::StrongType<Tag, T, Policies...>& value, FormatContext& ctx) const ->
      typename FormatContext::iterator {
    return underlying.format(value.value(), ctx);
  }
};

}  // namespace std
