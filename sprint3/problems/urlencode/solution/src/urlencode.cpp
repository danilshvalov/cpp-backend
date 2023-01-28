#include "urlencode.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

bool AnyOf(char value, std::string_view chars) {
    return std::any_of(chars.begin(), chars.end(), [&](char c) {
        return c == value;
    });
}

std::string UrlEncode(std::string_view str) {
    constexpr std::string_view reserved_symbols = "!\"#$%&'()*+,/:;=?@[]";
    std::stringstream ss;

    for (char chr : str) {
        int16_t symbol = chr;

        if (std::isspace(symbol)) {
            ss << '+';
        } else if (AnyOf(chr, reserved_symbols) || symbol < 32 || symbol >= 128) {
            ss << '%' << std::setfill('0') << std::setw(2) << std::hex
               << symbol;
        } else {
            ss << chr;
        }
    }

    return ss.str();
}
