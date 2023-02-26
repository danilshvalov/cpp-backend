#pragma once

#include <boost/algorithm/string.hpp>
#include <string>

namespace util {

bool IsSpace(char ch);

std::string TrimExtraSpaces(std::string str);

}  // namespace util
