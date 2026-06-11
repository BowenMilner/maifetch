#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

inline void expect_true(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

template <typename T, typename U>
inline void expect_eq(const T& actual, const U& expected, const std::string& message) {
    if (!(actual == expected)) {
        std::cerr << "FAIL: " << message << '\n';
        std::cerr << "  expected: " << expected << '\n';
        std::cerr << "  actual:   " << actual << '\n';
        std::exit(1);
    }
}
