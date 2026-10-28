// Лабораторна робота №2, пункт 1.2.7
// Два потоки типу void обмінюються результатами лише через promise/future.
// Глобальних змінних немає.

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
#include <utility>

void FirstThread(std::uint64_t n,
                 std::promise<std::uint64_t> nth,
                 std::promise<bool> startSecond,
                 std::promise<std::uint64_t> nthTimesTen) {
    nth.set_value(nthPrime(n));
    startSecond.set_value(true);
    nthTimesTen.set_value(nthPrime(n * 10));
}

void SecondThread(std::uint64_t n, std::future<bool> start) {
    start.get();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Корінь квадратний від n = " << std::sqrt(static_cast<double>(n)) << "\n";
}

std::uint64_t askNumber() {
    while (true) {
        std::cout << "Номер простого числа n = ";
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

int main() {
    setupConsole();

    try {
        std::uint64_t n = askNumber();

        std::promise<std::uint64_t> nthPromise;
        std::promise<std::uint64_t> nthTimesTenPromise;
        std::promise<bool> startPromise;

        std::future<std::uint64_t> nthFuture = nthPromise.get_future();
        std::future<std::uint64_t> nthTimesTenFuture = nthTimesTenPromise.get_future();
        std::future<bool> startFuture = startPromise.get_future();

        std::thread second(SecondThread, n, std::move(startFuture));
        std::thread first(FirstThread,
                          n,
                          std::move(nthPromise),
                          std::move(startPromise),
                          std::move(nthTimesTenPromise));

        std::cout << "Просте число №" << n << " = " << nthFuture.get() << "\n";
        std::cout << "Просте число №" << (n * 10) << " = " << nthTimesTenFuture.get() << "\n";

        first.join();
        second.join();
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 1;
    }
    return 0;
}
