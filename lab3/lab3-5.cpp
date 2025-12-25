#include <iostream>
#include <thread>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
std::queue<std::function<void()>> tasks; // task queue
std::mutex mtx; // mutex for queue
std::condition_variable cv; // condition variable
std::atomic<bool> done(false); // flag to stop threads
// Worker thread function
void worker(int id) {
while (!done) {
std::function<void()> task;
{
std::unique_lock<std::mutex> lock(mtx);

cv.wait(lock, []{ return !tasks.empty() || done; }); // wait for task
if (done && tasks.empty()) break;
task = tasks.front();
tasks.pop();
}
std::cout << "Worker " << id << " executing task\n";
task();
}
}
int main() {
const int THREADS = 3;
std::vector<std::thread> pool;
// create worker threads
for (int i = 0; i < THREADS; i++)
pool.emplace_back(worker, i+1);
// add tasks to the queue
for (int i = 1; i <= 6; i++) {
std::lock_guard<std::mutex> lock(mtx);
tasks.push([i]{ std::cout << "Task " << i << " done\n"; });
cv.notify_one(); // wake a worker
}
std::this_thread::sleep_for(std::chrono::seconds(1));
done = true; // signal threads to exit
cv.notify_all(); // wake all workers
for (auto& t : pool) t.join();
}
