#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mutex1, mutex2;

// Fixed: Both threads now lock mutexes in the same order
void thread1() {
    std::lock(mutex1, mutex2);  // Atomically lock both mutexes to prevent deadlock
    std::lock_guard<std::mutex> lock1(mutex1, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(mutex2, std::adopt_lock);
    
    std::cout << "Thread 1: Locked mutex1\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "Thread 1: Locked mutex2\n";
}

void thread2() {
    std::lock(mutex1, mutex2);  // Atomically lock both mutexes in same order as thread1
    std::lock_guard<std::mutex> lock1(mutex1, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(mutex2, std::adopt_lock);
    
    std::cout << "Thread 2: Locked mutex2\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "Thread 2: Locked mutex1\n";
}

int main() {
    std::thread t1(thread1);
    std::thread t2(thread2);
    
    t1.join();
    t2.join();
    
    return 0;
}