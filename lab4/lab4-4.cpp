// File: thread_scheduling.cpp
// Compile: g++ -o thread_scheduling thread_scheduling.cpp -std=c++17 -pthread

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <functional>

class ThreadInfo {
public:
    int thread_id;
    int priority;
    int burst_time;
    std::chrono::steady_clock::time_point arrival_time;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point completion_time;
    
    ThreadInfo(int id, int prio, int burst) 
        : thread_id(id), priority(prio), burst_time(burst) {
        arrival_time = std::chrono::steady_clock::now();
    }
    
    // Copy constructor
    ThreadInfo(const ThreadInfo& other) = default;
    
    // Assignment operator
    ThreadInfo& operator=(const ThreadInfo& other) = default;
};

// Custom comparator for priority queue
struct ThreadComparator {
    bool operator()(const ThreadInfo& a, const ThreadInfo& b) const {
        // Higher priority number = higher priority (reverse comparison for max heap)
        return a.priority < b.priority;
    }
};

class ThreadScheduler {
private:
    std::priority_queue<ThreadInfo, std::vector<ThreadInfo>, ThreadComparator> ready_queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::atomic<bool> running{true};
    std::atomic<int> submitted_threads{0};
    std::atomic<int> completed_threads{0};
    
public:
    ThreadScheduler() = default;
    
    void addThread(const ThreadInfo& thread_info) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            ready_queue.push(thread_info);
            submitted_threads++;
        }
        cv.notify_one();
    }
    
    void scheduler() {
        while (true) {
            std::unique_lock<std::mutex> lock(queue_mutex);
            
            // Wait until there's a thread to process or we're told to stop
            cv.wait(lock, [this] { 
                return !ready_queue.empty() || !running.load(); 
            });
            
            // Check if we should exit
            if (!running.load() && ready_queue.empty()) {
                break;
            }
            
            if (!ready_queue.empty()) {
                ThreadInfo current_thread = ready_queue.top();
                ready_queue.pop();
                lock.unlock();
                
                // Simulate thread execution
                current_thread.start_time = std::chrono::steady_clock::now();
                std::cout << "Executing Thread " << current_thread.thread_id 
                          << " (Priority: " << current_thread.priority << ")\n";
                
                std::this_thread::sleep_for(std::chrono::milliseconds(current_thread.burst_time * 100));
                
                current_thread.completion_time = std::chrono::steady_clock::now();
                
                auto turnaround_time = std::chrono::duration_cast<std::chrono::milliseconds>
                    (current_thread.completion_time - current_thread.arrival_time);
                
                std::cout << "Thread " << current_thread.thread_id 
                          << " completed. Turnaround time: " << turnaround_time.count() << "ms\n";
                
                // Update completed counter
                completed_threads++;
                
                // Notify waiters
                cv.notify_all();
            }
        }
        std::cout << "Scheduler stopped. Total threads completed: " << completed_threads.load() << "\n";
    }
    
    void stop() {
        running.store(false);
        cv.notify_all();
    }
    
    void waitForCompletion() {
        // Simple busy wait with short sleeps to avoid deadlock
        while (completed_threads.load() < submitted_threads.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        // Give a bit more time to ensure all output is flushed
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    int getCompletedThreadsCount() const {
        return completed_threads.load();
    }
    
    int getSubmittedThreadsCount() const {
        return submitted_threads.load();
    }
};

// Pthread-style thread attributes simulation with custom enum names
class ThreadAttributes {
public:
    // Use custom names to avoid conflicts with system constants
    enum SchedulingPolicy { POLICY_FIFO, POLICY_RR, POLICY_OTHER };
    enum ContentionScope { SCOPE_PROCESS, SCOPE_SYSTEM };
    
    SchedulingPolicy policy = POLICY_OTHER;
    ContentionScope scope = SCOPE_SYSTEM;
    int priority = 0;
    
    void setSchedulingPolicy(SchedulingPolicy pol) { policy = pol; }
    void setContentionScope(ContentionScope sc) { scope = sc; }
    void setPriority(int prio) { priority = prio; }
    
    void displayAttributes() const {
        std::cout << "Thread Attributes:\n";
        std::cout << "  Policy: " << (policy == POLICY_FIFO ? "FIFO" : 
                                    policy == POLICY_RR ? "Round Robin" : "Other") << "\n";
        std::cout << "  Scope: " << (scope == SCOPE_PROCESS ? "Process" : "System") << "\n";
        std::cout << "  Priority: " << priority << "\n";
    }
};

// Demo worker thread function
void workerThread(int id, int work_time) {
    std::cout << "Worker Thread " << id << " starting work for " << work_time << "ms\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(work_time));
    std::cout << "Worker Thread " << id << " completed work\n";
}

int main() {
    try {
        std::cout << "=== THREAD SCHEDULING DEMONSTRATION ===\n\n";
        
        // Demonstrate thread attributes
        ThreadAttributes attr;
        attr.setSchedulingPolicy(ThreadAttributes::POLICY_RR);
        attr.setContentionScope(ThreadAttributes::SCOPE_SYSTEM);
        attr.setPriority(5);
        attr.displayAttributes();
        
        std::cout << "\n=== THREAD SCHEDULER SIMULATION ===\n";
        
        ThreadScheduler scheduler;
        
        // Start scheduler in separate thread
        std::thread scheduler_thread(&ThreadScheduler::scheduler, &scheduler);
        
        // Create and schedule threads with different priorities
        scheduler.addThread(ThreadInfo(1, 3, 5));  // Medium priority
        scheduler.addThread(ThreadInfo(2, 1, 3));  // Low priority
        scheduler.addThread(ThreadInfo(3, 5, 4));  // High priority
        scheduler.addThread(ThreadInfo(4, 1, 2));  // Low priority
        
        // Wait for all threads to complete properly
        scheduler.waitForCompletion();
        
        // Stop the scheduler and wait for scheduler thread to finish
        scheduler.stop();
        if (scheduler_thread.joinable()) {
            scheduler_thread.join();
        }
        
        std::cout << "\n=== PTHREAD STYLE THREADS ===\n";
        
        // Create multiple worker threads
        std::vector<std::thread> workers;
        
        for (int i = 1; i <= 4; i++) {
            workers.emplace_back(workerThread, i, i * 200);
        }
        
        // Wait for all workers to complete
        for (auto& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        std::cout << "\nAll threads completed!\n";
        std::cout << "Scheduler processed " << scheduler.getCompletedThreadsCount() 
                  << " out of " << scheduler.getSubmittedThreadsCount() << " threads.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}