# Modules are added in strict topological order (Foundation → Domain → Analytics).
# Dependencies flow downward only — never upward.

# ── Foundation ────────────────────────────────────────────────────────────────
add_subdirectory(infra)
add_subdirectory(core)
add_subdirectory(math)

# ── Domain ────────────────────────────────────────────────────────────────────
add_subdirectory(instruments)
add_subdirectory(market)
add_subdirectory(curves)

# ── Analytics ─────────────────────────────────────────────────────────────────
add_subdirectory(pricing)
add_subdirectory(simulation)
add_subdirectory(risk)
add_subdirectory(portfolio)
