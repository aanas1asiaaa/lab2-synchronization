// Лабораторна робота №2, пункт 1.2.5
// n-те просте число через std::async: спочатку deferred, потім async.

#include "console.hpp"
#include "prime.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

using Clock = std::chrono::steady_clock;

double selectedFunction(int choice, std::uint64_t n) {
    double value = static_cast<double>(n);
    switch (choice) {
    case 1:
        return std::sqrt(value);
    case 2:
        return std::sin(value);
    case 3:
        return std::log(value);
    default:
        throw std::invalid_argument("Потрібен пункт 1, 2 або 3");
    }
}

const char* functionName(int choice) {
    switch (choice) {
    case 1:
        return "корінь квадратний";
    case 2:
        return "синус";
    case 3:
        return "натуральний логарифм";
    default:
        return "невідома функція";
    }
}

std::uint64_t askNumber(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            throw std::runtime_error("Введення завершено");
        }
        std::istringstream input(line);
        std::uint64_t value = 0;
        if (input >> value && input.eof() && value >= 1) {
            return value;
        }
        std::cout << "Введіть ціле число не менше 1.\n";
    }
}

int askChoice() {
    while (true) {
        std::cout << "Поки обчислюється просте число, можна дізнатися функцію від n.\n";
        std::cout << "1 - корінь квадратний, 2 - синус, 3 - натуральний логарифм: ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            throw std::runtime_error("Введення завершено");
        }
        std::istringstream input(line);
        int choice = 0;
        if (input >> choice && input.eof() && choice >= 1 && choice <= 3) {
            return choice;
        }
        std::cout << "Введіть 1, 2 або 3.\n";
    }
}

template <typename Future>
std::uint64_t takeResult(Future& future, const char* policy, Clock::duration& waited) {
    auto started = Clock::now();
    std::uint64_t prime = future.get();
    waited = Clock::now() - started;
    std::cout << policy << ": просте число = " << prime
              << ", очікування get() = "
              << std::chrono::duration_cast<std::chrono::milliseconds>(waited).count()
              << " мс\n";
    return prime;
}

int main() {
    setupConsole();

    try {
        std::uint64_t n = askNumber("Номер простого числа n = ");

        std::cout << "\nРежим std::launch::deferred: обчислення почнеться лише в get().\n";
        auto deferred = std::async(std::launch::deferred, nthPrime, n);
        int choice = askChoice();
        std::cout << functionName(choice) << "(n) = " << selectedFunction(choice, n) << "\n";
        Clock::duration deferredWait{};
        std::uint64_t deferredPrime = takeResult(deferred, "deferred", deferredWait);

        std::cout << "\nРежим std::launch::async: обчислення вже йде в іншому потоці.\n";
        auto asyncTask = std::async(std::launch::async, nthPrime, n);
        int secondChoice = askChoice();
        std::cout << functionName(secondChoice) << "(n) = " << selectedFunction(secondChoice, n) << "\n";
        Clock::duration asyncWait{};
        std::uint64_t asyncPrime = takeResult(asyncTask, "async", asyncWait);

        std::cout << "\nОбидва режими дали одне й те саме " << n
                  << "-те просте число: " << deferredPrime << "\n";
        if (deferredPrime != asyncPrime) {
            std::cout << "Розбіжність результатів: async повернув " << asyncPrime << "\n";
            return 1;
        }

        auto deferredMs = std::chrono::duration_cast<std::chrono::milliseconds>(deferredWait).count();
        auto asyncMs = std::chrono::duration_cast<std::chrono::milliseconds>(asyncWait).count();
        std::cout << "Очікування get() у deferred: " << deferredMs << " мс\n";
        std::cout << "Очікування get() у async: " << asyncMs << " мс\n";

        constexpr auto otherWork = std::chrono::milliseconds(1000);
        std::cout << "\nПорівняння, якщо між запуском і get() є інша робота "
                  << otherWork.count() << " мс.\n";

        auto deferredOverlap = std::async(std::launch::deferred, nthPrime, n);
        std::this_thread::sleep_for(otherWork);
        auto deferredGetStarted = Clock::now();
        std::uint64_t deferredOverlapPrime = deferredOverlap.get();
        auto deferredGetMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                 Clock::now() - deferredGetStarted)
                                 .count();

        auto asyncOverlap = std::async(std::launch::async, nthPrime, n);
        std::this_thread::sleep_for(otherWork);
        auto asyncGetStarted = Clock::now();
        std::uint64_t asyncOverlapPrime = asyncOverlap.get();
        auto asyncGetMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                              Clock::now() - asyncGetStarted)
                              .count();

        std::cout << "deferred після іншої роботи: get() = " << deferredGetMs
                  << " мс, число = " << deferredOverlapPrime << "\n";
        std::cout << "async після іншої роботи: get() = " << asyncGetMs
                  << " мс, число = " << asyncOverlapPrime << "\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 1;
    }
    return 0;
}
