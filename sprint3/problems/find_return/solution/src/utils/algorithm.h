#pragma once

#include <cmath>

namespace utils {
namespace consts {
    constexpr double epsilon = 0.01;
}

template<typename T>
bool AlmostEqual(T lhs, T rhs) {
    return std::abs(rhs - lhs) < consts::epsilon;
}

}  // namespace utils
