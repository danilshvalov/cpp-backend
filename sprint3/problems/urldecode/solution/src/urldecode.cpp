#include "urldecode.h"

#include <charconv>
#include <stdexcept>

std::string UrlDecode(std::string_view str) {
    constexpr size_t symbol_size = 2;
    std::string result;

    while (!str.empty()) {
        char first = str.front();
        str.remove_prefix(1);

        if (first == '+') {
            result += ' ';
            continue;
        } else if (first != '%') {
            result += first;
            continue;
        }

        if (str.size() < symbol_size) {
            throw std::invalid_argument(
                "Encoded character must be of the form %XY"
            );
        }

        uint16_t symbol;
        auto [ptr, ec] =
            std::from_chars(str.data(), str.data() + symbol_size, symbol, 16);

        if (ec != std::errc() || ptr - str.data() != symbol_size) {
            throw std::invalid_argument(
                "Encoded character must contain only hexadecimal values"
            );
        }

        result += static_cast<char>(symbol);
        str.remove_prefix(2);
    }

    return result;
}
