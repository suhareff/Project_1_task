#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

const int n = 5;  // количество птенцов
const int F = 10; // количество порций пищи в миске

std::mutex mtx;
std::condition_variable cv;
int food = F;

void chick(int id) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        while (food == 0) {
            cv.wait(lock); // ждём, пока мать наполнит миску
        }
        food--;
        std::cout << "Bird " << id << " eat. Food left: " << food << std::endl;
        if (food == 0) {
            std::cout << "Bird " << id << " calls Bird's mother." << std::endl;
            cv.notify_all(); // зовём мать
        }
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1)); // спим некоторое время
    }
}

void mother() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return food == 0; }); // ждём, пока миска опустеет
        food = F; // наполняем миску
        std::cout << "Mother filled the bowl. Portions: " << food << std::endl;
        cv.notify_all(); // уведомляем птенцов
        lock.unlock();
    }
}

int main() {
    std::vector<std::thread> chicks;
    for (int i = 0; i < n; ++i) {
        chicks.emplace_back(chick, i); // создаём потоки для птенцов
    }

    std::thread mother_thread(mother); // создаём поток для матери

    for (auto& chick : chicks) {
        chick.join(); // ждём завершения работы птенцов
    }

    mother_thread.join(); // ждём завершения работы матери

    return 0;
}
