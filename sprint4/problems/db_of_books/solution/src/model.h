#pragma once

#include <string>
#include <optional>

namespace model {

struct Book {
    size_t id = 0;
    std::string title;
    std::string author;
    int year;
    std::optional<std::string> isbn;
};

}  // namespace model
