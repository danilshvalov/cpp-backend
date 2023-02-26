#include "string.h"

namespace util {
bool IsSpace(char ch) {
    return std::isspace(static_cast<unsigned char>(ch));
}

std::string TrimExtraSpaces(std::string str) {
    boost::algorithm::trim(str);

    auto begin = std::find_if(str.begin(), str.end(), IsSpace);
    while (begin != str.end()) {
        const auto end = std::find_if_not(begin, str.end(), IsSpace);
        ++begin;
        str.erase(begin, end);
        begin = std::find_if(begin, str.end(), IsSpace);
    }

    return str;
}
}  // namespace util
