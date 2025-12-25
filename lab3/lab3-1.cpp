#include <iostream>
#include <thread>
#include <mutex>
int balance = 100; // shared bank account
std::mutex mtx; // mutex to protect shared resource
// Function to deposit money
void deposit(int amount) {
for (int i = 0; i < 5; i++) {
mtx.lock(); // lock before accessing balance
balance += amount; // critical section
std::cout << "Deposited " << amount << " | Balance = " << balance << "\n";
mtx.unlock(); // unlock after modification
std::this_thread::sleep_for(std::chrono::milliseconds(100)); // simulate work
}
}
// Function to withdraw money
void withdraw(int amount) {
for (int i = 0; i < 5; i++) {
mtx.lock(); // lock to prevent race condition
if (balance >= amount) {
balance -= amount; // safe modification
std::cout << "Withdrew " << amount << " | Balance = " << balance << "\n";
} else {
std::cout << "Withdrawal failed: insufficient funds!\n";
}
mtx.unlock(); // release lock
std::this_thread::sleep_for(std::chrono::milliseconds(120)); // simulate work
}
}
int main() {
std::thread t1(deposit, 50); // deposit thread
std::thread t2(withdraw, 30); // withdraw thread
t1.join(); // wait for deposit thread
t2.join(); // wait for withdraw thread
std::cout << "Final balance = " << balance << "\n"; // check final result
}
