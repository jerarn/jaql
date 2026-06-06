#pragma once

#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/sink.h>

#include <memory>
#include <string_view>

namespace jaql::infra {

/// @brief Returns or creates a named logger backed by the currently configured sink.
///
/// Loggers are registered in the spdlog global registry; a second call with the
/// same name returns the identical logger instance.
///
/// @param name  Unique identifier for the logger.
/// @return      Shared pointer to the named logger.
[[nodiscard]] auto get_logger(std::string_view name) -> std::shared_ptr<spdlog::logger>;

/// @brief Configures the global log level and optional output sink.
///
/// Affects only loggers created after this call. Must be called before the first
/// get_logger() invocation to ensure all loggers use the configured sink and level.
///
/// @param level  Minimum severity; messages below this level are suppressed.
/// @param sink   Output sink for new loggers; if nullptr, retains the current sink.
void configure_logging(spdlog::level::level_enum level,
                       std::shared_ptr<spdlog::sinks::sink> sink = nullptr);

}  // namespace jaql::infra
