jaql_add_module(
    infra
    SOURCES
        logger.cpp
    DEPS
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)
