// Лабораторна робота №2, пункт 1.2.1
// Синхронізація прапорцем, unique_lock і sleep_for(100 мс).

#include "console.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

std::queue<int> dataQueue;
std::mutex queueMutex;
bool inputDone = false;

bool isPrime(int value) {
    if (value <= 1) {
        return false;
    }
    if (value <= 3) {
        return true;
    }
    if (value % 2 == 0 || value % 3 == 0) {
        return false;
    }
    for (int divisor = 5; divisor <= value / divisor; divisor += 6) {
        if (value % divisor == 0 || value % (divisor + 2) == 0) {
            return false;
        }
    }
    return true;
}

bool parseInt(const std::string& line, int& value) {
    std::istringstream input(line);
    return static_cast<bool>(input >> value) && input.eof();
}

void DataPreparation() {
    std::cout << "Введіть цілі числа, кожне з нового рядка.\n";
    std::cout << "Порожній рядок або stop завершує введення.\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty() || line == "stop" || line == "стоп") {
            break;
        }
        int value = 0;
        if (!parseInt(line, value)) {
            std::cout << "Пропущено: '" << line << "' не є цілим числом.\n";
            continue;
        }
        std::lock_guard<std::mutex> lock(queueMutex);
        dataQueue.push(value);
    }

    std::lock_guard<std::mutex> lock(queueMutex);
    inputDone = true;
}

void DataProcessing() {
    std::unique_lock<std::mutex> lock(queueMutex);
    while (!inputDone) {
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        lock.lock();
    }

    std::queue<int> local;
    std::swap(local, dataQueue);
    lock.unlock();

    std::cout << "Прості числа:\n";
    bool found = false;
    while (!local.empty()) {
        int value = local.front();
        local.pop();
        if (isPrime(value)) {
            std::cout << value << "\n";
            found = true;
        }
    }
    if (!found) {
        std::cout << "(немає)\n";
    }
}

int main() {
    setupConsole();

    std::thread preparation(DataPreparation);
    preparation.detach();

    std::thread processing(DataProcessing);
    processing.join();
    return 0;
}
