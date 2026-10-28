// Лабораторна робота №2, пункт 1.2.2
// Три потоки Waits очікують notify_all від Awake.
// Перший сигнал іде при i == 0, другий — після i = 1.

#include "console.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mutex;
std::condition_variable event;
int i = 0;
std::atomic<int> waitingCount{0};

void Waits(int id) {
    std::unique_lock<std::mutex> lock(mutex);
    std::cout << "Потік " << id << " входить у стан очікування\n";
    waitingCount.fetch_add(1);
    event.wait(lock, [] { return i == 1; });
    std::cout << "Потік " << id << ": очікування завершене\n";
}

void Awake() {
    while (waitingCount.load() < 3) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    {
        std::lock_guard<std::mutex> lock(mutex);
        std::cout << "Awake: починаю повідомляти решту потоків (i = " << i << ")\n";
        event.notify_all();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    {
        std::lock_guard<std::mutex> lock(mutex);
        i = 1;
        std::cout << "Awake: повторне повідомлення (i = " << i << ")\n";
        event.notify_all();
    }
}

int main() {
    setupConsole();

    std::thread first(Waits, 1);
    std::thread second(Waits, 2);
    std::thread third(Waits, 3);
    std::thread awake(Awake);

    awake.join();
    first.join();
    second.join();
    third.join();
    return 0;
}
