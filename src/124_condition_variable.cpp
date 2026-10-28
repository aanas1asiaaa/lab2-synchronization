// Лабораторна робота №2, пункт 1.2.4
// Та сама черга, що в 1.2.1, але очікування через condition_variable.
// Предикат wait() перевіряє, чи черга порожня.

#include "console.hpp"

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>

std::queue<int> dataQueue;
std::mutex queueMutex;
std::condition_variable dataReady;
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
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            dataQueue.push(value);
        }
        dataReady.notify_one();
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        inputDone = true;
    }
    dataReady.notify_one();
}

void DataProcessing() {
    bool headerPrinted = false;
    while (true) {
        int value = 0;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            dataReady.wait(lock, [] { return !dataQueue.empty() || inputDone; });
            if (dataQueue.empty()) {
                break;
            }
            value = dataQueue.front();
            dataQueue.pop();
        }
        if (!isPrime(value)) {
            continue;
        }
        if (!headerPrinted) {
            std::cout << "Прості числа:\n";
            headerPrinted = true;
        }
        std::cout << value << "\n";
    }
    if (!headerPrinted) {
        std::cout << "Прості числа:\n(немає)\n";
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
