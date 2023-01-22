#pragma once

#include <algorithm>

namespace utils {

template<typename T>
bool IsIntersect(const T& lhs, const T& rhs) {
    return std::any_of(lhs.begin(), lhs.end(), [&](const auto& value) {
        return rhs.contains(value);
    });
}

}  // namespace utils
