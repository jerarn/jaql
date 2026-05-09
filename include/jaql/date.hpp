#pragma once

#include <chrono>
#include <string>

namespace jaql {

struct Date {
    int year;
    unsigned int month;
    unsigned int day;

    [[nodiscard]] std::chrono::sys_days to_sys_days() const;
    [[nodiscard]] std::string to_string() const;
};

[[nodiscard]] int days_between(const Date& start, const Date& end);

} // namespace jaql
