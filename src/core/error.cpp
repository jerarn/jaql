#include <jaql/core/error.hpp>

#include <format>
#include <string>

namespace jaql::core {

auto to_string(Code code) -> std::string_view {
  switch (code) {
    case Code::Ok:
      return "Ok";
    case Code::InvalidArgument:
      return "InvalidArgument";
    case Code::OutOfRange:
      return "OutOfRange";
    case Code::InvalidDate:
      return "InvalidDate";
    case Code::NumericalFailure:
      return "NumericalFailure";
    case Code::NotFound:
      return "NotFound";
    case Code::ParseError:
      return "ParseError";
    case Code::InternalError:
      return "InternalError";
  }
  return "Unknown";
}

Error::Error(Code code, std::string message, std::source_location location)
    : code_{code}, message_{std::move(message)}, location_{location} {}

auto Error::what() const -> std::string {
  return std::format("[{}] {} ({}:{})", to_string(code_), message_, location_.file_name(),
                     location_.line());
}

}  // namespace jaql::core
