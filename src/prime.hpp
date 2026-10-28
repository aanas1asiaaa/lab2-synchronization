#pragma once

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

// Повертає n-те просте число. Нумерація з одиниці: 1 -> 2, 2 -> 3, 3 -> 5.
inline std::uint64_t nthPrime(std::uint64_t n) {
    if (n == 0) {
        throw std::invalid_argument("Номер простого числа починається з 1");
    }
    if (n == 1) {
        return 2;
    }

    double estimate = static_cast<double>(n);
    double upper = (n < 6) ? 15.0 : estimate * (std::log(estimate) + std::log(std::log(estimate)));
    auto limit = static_cast<std::uint64_t>(upper) + 32;

    for (;;) {
        std::vector<char> composite(static_cast<std::size_t>(limit) + 1, 0);
        std::uint64_t count = 0;
        for (std::uint64_t p = 2; p <= limit; ++p) {
            if (composite[static_cast<std::size_t>(p)]) {
                continue;
            }
            ++count;
            if (count == n) {
                return p;
            }
            if (p <= limit / p) {
                for (std::uint64_t q = p * p; q <= limit; q += p) {
                    composite[static_cast<std::size_t>(q)] = 1;
                }
            }
        }
        limit = limit * 2 + 32;
    }
}
