#include <jaql/infra/logger.hpp>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <mutex>
#include <string>

namespace jaql::infra {

namespace {

struct LoggingConfig {
  std::shared_ptr<spdlog::sinks::sink> sink;
  spdlog::level::level_enum level{spdlog::level::info};
  std::mutex mutex;
};

auto global_config() -> LoggingConfig& {
  static LoggingConfig config;
  return config;
}

auto default_sink() -> std::shared_ptr<spdlog::sinks::sink> {
  static auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  return sink;
}

}  // namespace

auto get_logger(std::string_view name) -> std::shared_ptr<spdlog::logger> {
  std::string name_str(name);

  // Fast path: logger already registered (spdlog registry is internally thread-safe).
  if (auto existing = spdlog::get(name_str)) {
    return existing;
  }

  auto& config = global_config();
  std::scoped_lock lock(config.mutex);

  // Double-checked locking: re-check after acquiring the lock.
  if (auto existing = spdlog::get(name_str)) {
    return existing;
  }

  auto active_sink = config.sink ? config.sink : default_sink();
  auto logger = std::make_shared<spdlog::logger>(name_str, active_sink);
  logger->set_level(config.level);
  spdlog::register_logger(logger);
  return logger;
}

void configure_logging(spdlog::level::level_enum level, std::shared_ptr<spdlog::sinks::sink> sink) {
  auto& config = global_config();
  std::scoped_lock lock(config.mutex);
  config.level = level;
  if (sink) {
    config.sink = std::move(sink);
  }
}

}  // namespace jaql::infra
