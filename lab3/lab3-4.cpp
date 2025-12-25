#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
const int N = 5; // number of philosophers
std::mutex forks[N]; // forks are shared resources
void philosopher(int id) {
int left = id;
int right = (id + 1) % N;
for (int i = 0; i < 3; i++) {
std::cout << "Philosopher " << id << " is thinking\n";
std::this_thread::sleep_for(std::chrono::milliseconds(200));
// attempt to pick both forks (may deadlock without std::lock)
std::lock(forks[left], forks[right]);
std::lock_guard<std::mutex> lock1(forks[left], std::adopt_lock);
std::lock_guard<std::mutex> lock2(forks[right], std::adopt_lock);
std::cout << "Philosopher " << id << " is eating\n";
std::this_thread::sleep_for(std::chrono::milliseconds(300));
}
}
int main() {
std::vector<std::thread> phils;
for (int i = 0; i < N; i++) phils.emplace_back(philosopher, i);
for (auto& t : phils) t.join();
}
