// Лабораторна робота №2, пункт 1.2.3
// Три потоки чекають, Notify будить лише один через notify_one().

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

void WaitingThread(int id) {
    std::unique_lock<std::mutex> lock(mutex);
    std::cout << "Потік " << id << " входить у стан очікування\n";
    waitingCount.fetch_add(1);
    event.wait(lock, [] { return i == 1; });
    std::cout << "Повідомлення з потоку " << id << "\n";
}

void Notify() {
    while (waitingCount.load() < 3) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << "Notify: виклик notify_one()\n";
    event.notify_one();
    i = 1;
    std::cout << "Notify: змінній i присвоєно 1\n";
}

int main() {
    setupConsole();

    std::thread thread1(WaitingThread, 1);
    std::thread thread2(WaitingThread, 2);
    std::thread thread3(WaitingThread, 3);
    std::thread notify(Notify);

    // Головний потік чекає лише Notify.
    // Два з трьох потоків після notify_one() ніколи не вийдуть з wait(),
    // тому їх від'єднано: join() на них заблокував би програму.
    thread1.detach();
    thread2.detach();
    thread3.detach();

    notify.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return 0;
}
