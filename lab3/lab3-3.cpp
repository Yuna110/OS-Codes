#include <iostream>
#include <thread>
#include <shared_mutex>
std::shared_mutex rwLock; // allows multiple readers or single writer
int sharedData = 0; // shared data
// Reader function
void reader(int id) {
for (int i = 0; i < 3; i++) {
rwLock.lock_shared(); // shared lock allows multiple readers
std::cout << "Reader " << id << " read data = " << sharedData << "\n";
rwLock.unlock_shared();
std::this_thread::sleep_for(std::chrono::milliseconds(200));
}
}
// Writer function
void writer(int id) {
for (int i = 0; i < 3; i++) {
rwLock.lock(); // exclusive lock for writing
sharedData += 10;
std::cout << "Writer " << id << " updated data = " << sharedData << "\n";
rwLock.unlock();
std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

}
int main() {
std::thread r1(reader, 1), r2(reader, 2), w1(writer, 1);
r1.join(); r2.join(); w1.join();
}
