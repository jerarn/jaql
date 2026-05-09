#include "jaql/date.hpp"

#include <iomanip>
#include <sstream>

namespace jaql {

std::chrono::sys_days Date::to_sys_days() const {
    return std::chrono::year(year) / std::chrono::month(month) / std::chrono::day(day);
}

std::string Date::to_string() const {
    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(4) << year << "-" << std::setw(2) << month << "-" << std::setw(2) << day;
    return stream.str();
}

int days_between(const Date& start, const Date& end) {
    return static_cast<int>((end.to_sys_days() - start.to_sys_days()).count());
}

} // namespace jaql
