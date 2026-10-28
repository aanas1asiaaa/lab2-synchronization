// Лабораторна робота №2, пункт 1.2.6
// Черга packaged_task і черга номерів n. Окремий потік виконує завдання.

#include "console.hpp"
#include "prime.hpp"

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <future>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

std::mutex dequeMutex;
std::condition_variable taskReady;
std::deque<std::packaged_task<std::uint64_t()>> tasks;
std::deque<std::uint64_t> numbers;
bool stopWorker = false;

void Worker() {
    while (true) {
        std::packaged_task<std::uint64_t()> task;
        std::uint64_t n = 0;
        {
            std::unique_lock<std::mutex> lock(dequeMutex);
            taskReady.wait(lock, [] { return stopWorker || !tasks.empty(); });
            if (tasks.empty()) {
                return;
            }
            task = std::move(tasks.front());
            tasks.pop_front();
            n = numbers.front();
            numbers.pop_front();
        }

        std::cout << "Робочий потік узяв завдання n = " << n << "\n";
        task();
    }
}

bool parseNumber(const std::string& line, std::uint64_t& value) {
    std::istringstream input(line);
    return static_cast<bool>(input >> value) && input.eof() && value >= 1;
}

int main() {
    setupConsole();

    std::thread worker(Worker);
    std::vector<std::future<std::uint64_t>> results;
    std::vector<std::uint64_t> asked;

    std::cout << "Введіть номери простих чисел. stop або порожній рядок завершує опитування.\n";
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty() || line == "stop" || line == "стоп") {
            break;
        }

        std::uint64_t n = 0;
        if (!parseNumber(line, n)) {
            std::cout << "Пропущено: потрібне ціле число не менше 1.\n";
            continue;
        }

        std::packaged_task<std::uint64_t()> task([n] { return nthPrime(n); });
        results.push_back(task.get_future());
        asked.push_back(n);

        {
            std::lock_guard<std::mutex> lock(dequeMutex);
            tasks.push_back(std::move(task));
            numbers.push_back(n);
        }
        taskReady.notify_one();
    }

    {
        std::lock_guard<std::mutex> lock(dequeMutex);
        stopWorker = true;
    }
    taskReady.notify_one();
    worker.join();

    std::cout << "Результати:\n";
    for (std::size_t index = 0; index < results.size(); ++index) {
        std::cout << "Просте число №" << asked[index] << " = " << results[index].get() << "\n";
    }
    return 0;
}
