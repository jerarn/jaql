# Design Principles

This document defines the engineering philosophy that governs all JAQL code. These
principles exist to make a long-lived codebase correct, readable, and maintainable
over a sustained engineering timeline.

---

## 1. Error Handling — `Result<T>` over Exceptions

### Principle

JAQL uses `Result<T>` (an alias for `tl::expected<T, jaql::Error>`) as the primary error
propagation mechanism for operations that can fail in well-defined ways. C++ exceptions
are reserved for truly exceptional, unrecoverable situations (programmer errors, system
failures).

### Rationale

Exceptions have several costs in quantitative finance code:
- They obscure error paths, making it hard to reason about failure modes.
- They impose overhead on hot paths even when not thrown (exception table metadata,
  deoptimization in some ABIs).
- They discourage callers from handling errors — a `double price(...)` that throws is
  silent about its failure modes until runtime.

`Result<T>` forces callers to acknowledge that an operation can fail. The compiler
enforces it.

### Pattern

```cpp
// Function signature makes error possibility explicit
auto discount_factor(YearFraction t, Rate r) noexcept -> Result<double>;

// Caller must handle both cases
auto df_result = discount_factor(t, r);
if (!df_result) {
    log_error(df_result.error().message);
    return tl::unexpected(df_result.error());  // propagate
}
double df = *df_result;

// Or use the monadic interface (C++23 std::expected / tl::expected compatible)
auto pv = discount_factor(t, r)
    .and_then([&](double df) { return compute_pv(cashflows, df); })
    .map([](double pv) { return PricingResult{pv}; });
```

### When to Use Exceptions

Exceptions are appropriate for:
- **Programmer errors** detected at startup (misconfigured engine, invalid module init).
- **Unrecoverable system failures** (out of memory, file I/O failure on required data).
- **Constructor failures** where `Result` cannot be returned — use a named factory
  function instead and keep constructors non-throwing.

Exceptions are **not** appropriate for:
- Missing market data (return `Result<T>` with `Error::Code::MarketDataNotFound`).
- Curve extrapolation out of range (return `Result<T>`).
- Numerical convergence failure (return `Result<T>`).

---

## 2. Strong Types — Preventing Unit Confusion

### Principle

JAQL uses `StrongType<Tag, T>` to wrap primitive values in domain-meaningful types. A
`Rate` is not interchangeable with a `Spread` even though both are `double` at runtime.

### Rationale

Unit confusion is one of the most common and costly bugs in quantitative finance. A
function taking `(double rate, double spread, double notional)` is trivially misused.
Strong types turn this into a compile-time error.

### Pattern

```cpp
using Rate     = jaql::StrongType<struct RateTag, double>;
using Spread   = jaql::StrongType<struct SpreadTag, double>;
using Notional = jaql::StrongType<struct NotionalTag, double>;

// Function signature is self-documenting and enforced
auto price_bond(Rate coupon, Spread credit_spread, Notional notional)
    -> Result<PricingResult>;

// Correct call — explicit at the call site
auto result = price_bond(Rate{0.05}, Spread{0.02}, Notional{1'000'000.0});

// Wrong call — does not compile
auto result = price_bond(0.05, 0.02, 1'000'000.0);  // error: no matching call
auto result = price_bond(Spread{0.02}, Rate{0.05}, Notional{1'000'000.0});  // error
```

### `StrongType<Tag, T>` Design Constraints

- Zero overhead: `StrongType<Tag, double>` has the same size and alignment as `double`.
- Explicit construction only: no implicit conversion from `T`.
- Arithmetic operators are opt-in via CRTP mixin policies (addable, scalable, comparable).
- Hash and comparison support for use in containers.

---

## 3. Ownership — RAII and Explicit Transfer

### Principle

Every resource has a single, explicit owner at all times. Ownership transfer is always
visible at the call site.

### Ownership Model

| Smart pointer       | Use case                                              |
|---------------------|-------------------------------------------------------|
| `std::unique_ptr<T>`| Default for dynamically allocated objects. Sole owner.|
| `std::shared_ptr<T>`| Only when shared ownership is genuinely required.     |
| `T*` (raw, non-owning) | Non-owning observer/reference. Never deletes.      |
| `std::span<T>`      | Non-owning view of a contiguous sequence.             |
| `std::string_view`  | Non-owning view of a string.                          |

### Rules

1. **Never use `new` or `delete` directly.** Use `std::make_unique`, `std::make_shared`,
   or stack allocation.
2. **`std::shared_ptr` requires justification.** If you cannot name the multiple owners,
   `unique_ptr` is correct.
3. **Raw pointers signal non-ownership.** A raw pointer `T*` in a function signature
   means "I observe this, I do not own it."
4. **Factory functions hide constructors.** Complex object construction uses named
   factory functions that return `unique_ptr` or `Result<unique_ptr<T>>`, keeping
   constructors simple and non-throwing.

```cpp
// Good: explicit ownership transfer
std::unique_ptr<YieldCurve> curve = BootstrappedCurve::build(instruments, market);

// Good: non-owning reference for observation
void price(const Instrument& instrument, const YieldCurve* curve);

// Bad: unclear ownership, potential leak
YieldCurve* curve = new BootstrappedCurve(instruments, market);
```

---

## 4. Value Semantics

### Principle

Prefer value types over reference types. Copy and move semantics must be correct and
efficient before reference semantics are considered.

### Rationale

Value types are simpler to reason about: there is no aliasing, no shared mutation, and
no lifetime to track. For small financial primitives (`Rate`, `Cashflow`, `Quote<T>`),
value semantics are both correct and efficient.

For larger objects (curves, market data snapshots), move semantics and `std::unique_ptr`
provide value-like semantics without copying cost.

### Guideline

- Small types (`sizeof` ≤ 16 bytes): pass by value.
- Medium types: pass by const reference, return by value.
- Large types owned by caller: pass by const reference or `std::span`.
- Objects with ownership: transfer via `std::unique_ptr` move.

---

## 5. Immutability and `const`-Correctness

### Principle

Make objects immutable by default. A value that does not need to change should not be
allowed to change.

### Rules

1. All member functions that do not modify state must be `const`.
2. Local variables that are not reassigned must be `const` or `constexpr`.
3. Function parameters that are not modified must be `const`.
4. Prefer `const` member data over non-`const` where the type is immutable after
   construction (e.g., a curve's tenor structure).

```cpp
// Good
const auto discount = curve.discount_factor(maturity);
constexpr double spread_bump = 0.0001;

// Bad — unnecessary mutability
auto discount = curve.discount_factor(maturity);  // discount never changes
double spread_bump = 0.0001;
```

---

## 6. Template Guidelines

### When to Use Templates

Templates are appropriate when:
- The behaviour is truly generic across unrelated types (e.g., `StrongType<Tag, T>`).
- The type must be resolved at compile time for performance (e.g., a pricing engine
  templated on a specific RNG type in the hot simulation path).
- C++23 Concepts can constrain the template cleanly with a readable error message.

### When NOT to Use Templates

Avoid templates when:
- Virtual dispatch on a small number of types is sufficient and simpler.
- The template adds complexity without measurable performance benefit.
- The resulting error messages are incomprehensible.

### Concepts Over SFINAE

Always use C++23 Concepts (`requires`) instead of SFINAE for template constraints.
Concepts produce readable error messages and serve as documentation.

```cpp
// Good: Concept clearly states the requirement
template <typename Curve>
    requires YieldCurveType<Curve>
auto price_bond(const Bond& bond, const Curve& curve) -> Result<PricingResult>;

// Bad: SFINAE — cryptic errors, unreadable at call site
template <typename Curve, typename = std::enable_if_t<is_yield_curve_v<Curve>>>
auto price_bond(const Bond& bond, const Curve& curve) -> Result<PricingResult>;
```

---

## 7. API Design Philosophy

### Named Factory Functions Over Complex Constructors

Complex initialization logic belongs in factory functions, not constructors. Constructors
should be trivial enough that they cannot fail.

```cpp
// Good: factory communicates intent, can return Result<T>
auto curve = BootstrappedCurve::from_deposit_rates(tenors, rates, convention);
// → Returns Result<std::unique_ptr<YieldCurve>>

// Bad: complex constructor, can throw, no error information
BootstrappedCurve curve(tenors, rates, convention);  // might throw?
```

### `[[nodiscard]]` on All Result-Returning Functions

Any function returning `Result<T>` or a meaningful value must be marked `[[nodiscard]]`.
Discarding a `Result<T>` is almost always a bug.

```cpp
[[nodiscard]] auto discount_factor(YearFraction t) const noexcept -> Result<double>;
```

### Explicit Is Better Than Implicit

- All single-argument constructors are `explicit` unless implicit conversion is the
  intended design.
- No operator overloads that change type identity implicitly.
- No default arguments that hide required information.

---

## 8. Memory Management Philosophy

### Rules

1. No dynamic allocation in hot pricing paths unless unavoidable and profiled.
2. Containers (`std::vector`, `std::array`) are preferred over manual allocation.
3. For high-frequency object construction (Monte Carlo paths), arena allocators will be
   introduced in Phase 4 with a stable allocator abstraction.
4. `std::pmr::polymorphic_allocator` is the planned mechanism for allocator abstraction
   when arena allocation is introduced.

---

## 9. Threading Philosophy

JAQL's initial implementation is single-threaded at the module level. The architecture
supports future parallelism through the following constraints:

1. **No global mutable state.** All state is owned by explicitly-constructed objects.
2. **Thread safety by construction.** Objects shared between threads must be either
   immutable or use explicit synchronization.
3. **No static mutable variables** in module code (only in infrastructure such as the
   logger singleton).
4. **Pricing engines are stateless.** A `PricingEngine` holds configuration, not
   intermediate state, making it safe to use from multiple threads.

Future parallelism will be introduced via a task queue in `infra`, dispatching
independent pricing jobs across a thread pool, with no changes required to engine code.

---

## 10. Compile-Time vs Runtime Tradeoffs

| Decision                         | Approach                                    |
|----------------------------------|---------------------------------------------|
| Mathematical constants           | `constexpr double` — always compile-time    |
| Type safety (units, currencies)  | `StrongType<>` — zero runtime cost          |
| Pricing algorithm selection      | Template parameter — compile-time dispatch  |
| Interpolation method selection   | Template parameter or concept — compile-time|
| Instrument type selection        | Runtime polymorphism only if necessary      |
| Error handling                   | `Result<T>` — no exception overhead         |
| Debug assertions                 | `JAQL_ASSERT` — stripped in release builds  |
| Precondition checking            | `JAQL_EXPECTS` — configurable at build time |
