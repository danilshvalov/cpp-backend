#pragma once

#include <random>

namespace utils {

template<typename T>
T GenerateRandomNumber(T lhs, T rhs) {
    std::random_device device;
    std::default_random_engine engine(device());
    if constexpr (std::is_integral_v<T>) {
        std::uniform_int_distribution<T> distribution(lhs, rhs);
        return distribution(engine);
    } else {
        std::uniform_real_distribution<T> distribution(lhs, rhs);
        return distribution(engine);
    }
}

}  // namespace utils
