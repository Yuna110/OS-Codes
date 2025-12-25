#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
std::queue<int> buffer; // shared buffer

const unsigned int MAX = 5; // max buffer size
std::mutex mtx; // mutex to protect buffer
std::condition_variable cv; // condition variable for sync
// Producer function
void producer() {
for (int i = 1; i <= 10; i++) {
std::unique_lock<std::mutex> lock(mtx); // lock buffer
cv.wait(lock, [] { return buffer.size() < MAX; }); // wait if buffer full
buffer.push(i); // produce item
std::cout << "Produced: " << i << "\n";
cv.notify_all(); // notify consumers
lock.unlock();
std::this_thread::sleep_for(std::chrono::milliseconds(100)); // simulate production time
}
}
// Consumer function
void consumer() {
for (int i = 1; i <= 10; i++) {
std::unique_lock<std::mutex> lock(mtx); // lock buffer
cv.wait(lock, [] { return !buffer.empty(); }); // wait if buffer empty
int item = buffer.front(); // consume item
buffer.pop();
std::cout << "Consumed: " << item << "\n";
cv.notify_all(); // notify producer
lock.unlock();
std::this_thread::sleep_for(std::chrono::milliseconds(150)); // simulate consumption time
}
}
int main() {
std::thread t1(producer); // producer thread
std::thread t2(consumer); // consumer thread
t1.join(); // wait for producer to finish
t2.join(); // wait for consumer to finish
}
