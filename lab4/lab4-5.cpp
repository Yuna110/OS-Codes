// File: multiprocessor_scheduling.cpp
// Compile: g++ -o multiprocessor_scheduling multiprocessor_scheduling.cpp -std=c++17 -pthread

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <random>
#include <algorithm>
#include <climits>
#include <memory>

class Task {
public:
    int task_id;
    int burst_time;
    int preferred_cpu;
    std::chrono::steady_clock::time_point arrival_time;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point completion_time;
    
    Task(int id, int burst, int cpu = -1) 
        : task_id(id), burst_time(burst), preferred_cpu(cpu) {
        arrival_time = std::chrono::steady_clock::now();
    }
};

class CPUCore {
public:
    int core_id;
    std::queue<Task> local_queue;
    std::mutex queue_mutex;
    std::atomic<bool> is_busy{false};
    std::atomic<int> load{0};
    
    CPUCore(int id) : core_id(id) {}
    
    // Delete copy constructor and assignment operator due to mutex
    CPUCore(const CPUCore&) = delete;
    CPUCore& operator=(const CPUCore&) = delete;
    
    void addTask(const Task& task) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        local_queue.push(task);
        load++;
    }
    
    bool getTask(Task& task) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        if (!local_queue.empty()) {
            task = local_queue.front();
            local_queue.pop();
            load--;
            return true;
        }
        return false;
    }
    
    int getQueueSize() {
        std::lock_guard<std::mutex> lock(queue_mutex);
        return static_cast<int>(local_queue.size());
    }
    
    bool isEmpty() {
        std::lock_guard<std::mutex> lock(queue_mutex);
        return local_queue.empty();
    }
};

class MultiProcessorScheduler {
private:
    std::vector<std::unique_ptr<CPUCore>> cores;
    std::queue<Task> global_queue;
    std::mutex global_mutex;
    std::condition_variable cv;
    std::atomic<bool> running{true};
    std::atomic<int> active_tasks{0};
    std::atomic<int> completed_tasks{0};
    int num_cores;
    
    // Load balancing parameters
    static constexpr int LOAD_BALANCE_THRESHOLD = 2;
    static constexpr int MIGRATION_COST = 5; // milliseconds
    
public:
    MultiProcessorScheduler(int cores_count) : num_cores(cores_count) {
        cores.reserve(cores_count);
        for (int i = 0; i < cores_count; i++) {
            cores.push_back(std::make_unique<CPUCore>(i));
        }
    }
    
    void addTask(const Task& task) {
        if (task.preferred_cpu >= 0 && task.preferred_cpu < num_cores) {
            // Processor affinity - try preferred CPU first
            cores[task.preferred_cpu]->addTask(task);
        } else {
            // Global queue for load balancing
            std::lock_guard<std::mutex> lock(global_mutex);
            global_queue.push(task);
        }
        active_tasks++;
        cv.notify_all();
    }
    
    void cpuScheduler(int core_id) {
        std::cout << "CPU Core " << core_id << " scheduler started\n";
        
        while (running.load() || active_tasks.load() > 0) {
            Task current_task(0, 0);
            bool has_task = false;
            
            // Try to get task from local queue first (processor affinity)
            if (cores[core_id]->getTask(current_task)) {
                has_task = true;
            }
            // Try global queue
            else {
                std::lock_guard<std::mutex> lock(global_mutex);
                if (!global_queue.empty()) {
                    current_task = global_queue.front();
                    global_queue.pop();
                    has_task = true;
                }
            }
            
            // Work stealing - try to steal from other cores
            if (!has_task) {
                has_task = workStealing(core_id, current_task);
            }
            
            if (has_task) {
                executeTask(core_id, current_task);
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        
        std::cout << "CPU Core " << core_id << " scheduler stopped\n";
    }
    
    bool workStealing(int core_id, Task& stolen_task) {
        // Find the most loaded core
        int max_load = 0;
        int victim_core = -1;
        
        for (int i = 0; i < num_cores; i++) {
            if (i != core_id && cores[i]->getQueueSize() > max_load + LOAD_BALANCE_THRESHOLD) {
                max_load = cores[i]->getQueueSize();
                victim_core = i;
            }
        }
        
        if (victim_core != -1 && cores[victim_core]->getTask(stolen_task)) {
            std::cout << "Core " << core_id << " stole task " << stolen_task.task_id 
                      << " from Core " << victim_core << "\n";
            return true;
        }
        
        return false;
    }
    
    void executeTask(int core_id, Task& task) {
        cores[core_id]->is_busy = true;
        task.start_time = std::chrono::steady_clock::now();
        
        std::cout << "Core " << core_id << " executing Task " << task.task_id 
                  << " (Burst: " << task.burst_time << "ms)\n";
        
        // Simulate task execution
        std::this_thread::sleep_for(std::chrono::milliseconds(task.burst_time));
        
        task.completion_time = std::chrono::steady_clock::now();
        
        auto turnaround_time = std::chrono::duration_cast<std::chrono::milliseconds>
            (task.completion_time - task.arrival_time);
        
        std::cout << "Core " << core_id << " completed Task " << task.task_id 
                  << " (Turnaround: " << turnaround_time.count() << "ms)\n";
        
        cores[core_id]->is_busy = false;
        active_tasks--;
        completed_tasks++;
    }
    
    void loadBalancer() {
        while (running.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Check load imbalance
            int min_load = INT_MAX;
            int max_load = 0;
            int min_core = -1;
            int max_core = -1;
            
            for (int i = 0; i < num_cores; i++) {
                int load = cores[i]->getQueueSize();
                if (load < min_load) {
                    min_load = load;
                    min_core = i;
                }
                if (load > max_load) {
                    max_load = load;
                    max_core = i;
                }
            }
            
            // Migrate tasks if imbalance is significant
            if (max_load - min_load > LOAD_BALANCE_THRESHOLD && max_core != -1 && min_core != -1) {
                Task migrated_task(0, 0);
                if (cores[max_core]->getTask(migrated_task)) {
                    cores[min_core]->addTask(migrated_task);
                    std::cout << "Load Balancer: Migrated Task " << migrated_task.task_id 
                              << " from Core " << max_core << " to Core " << min_core << "\n";
                }
            }
        }
    }
    
    void waitForCompletion() {
        // Wait until all tasks are completed
        while (active_tasks.load() > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    void displayStats() {
        std::cout << "\n=== CPU CORE STATISTICS ===\n";
        for (int i = 0; i < num_cores; i++) {
            std::cout << "Core " << i << ": Queue Size = " << cores[i]->getQueueSize()
                      << ", Busy = " << (cores[i]->is_busy.load() ? "Yes" : "No") << "\n";
        }
        std::cout << "Active Tasks: " << active_tasks.load() << "\n";
        std::cout << "Completed Tasks: " << completed_tasks.load() << "\n";
    }
    
    void stop() {
        running = false;
        cv.notify_all();
    }
};

// NUMA-aware scheduler simulation
class NUMAScheduler {
private:
    struct NUMANode {
        int node_id;
        std::vector<int> cpu_cores;
        int memory_latency; // Access latency in nanoseconds
        
        NUMANode(int id, std::vector<int> cores, int latency) 
            : node_id(id), cpu_cores(std::move(cores)), memory_latency(latency) {}
    };
    
    std::vector<NUMANode> numa_nodes;
    
public:
    NUMAScheduler() {
        // Simulate 2 NUMA nodes
        numa_nodes.emplace_back(0, std::vector<int>{0, 1}, 100); // Local access
        numa_nodes.emplace_back(1, std::vector<int>{2, 3}, 300); // Remote access
    }
    
    int selectOptimalCore(int preferred_node = -1) {
        if (preferred_node >= 0 && preferred_node < static_cast<int>(numa_nodes.size())) {
            // Return first available core from preferred NUMA node
            const auto& node = numa_nodes[preferred_node];
            if (!node.cpu_cores.empty()) {
                return node.cpu_cores[0];
            }
        }
        
        // Find node with lowest memory latency that has available cores
        int best_node = 0;
        int min_latency = numa_nodes[0].memory_latency;
        
        for (size_t i = 1; i < numa_nodes.size(); i++) {
            if (numa_nodes[i].memory_latency < min_latency) {
                min_latency = numa_nodes[i].memory_latency;
                best_node = static_cast<int>(i);
            }
        }
        
        return numa_nodes[best_node].cpu_cores[0];
    }
    
    void displayNUMATopology() {
        std::cout << "\n=== NUMA TOPOLOGY ===\n";
        for (const auto& node : numa_nodes) {
            std::cout << "NUMA Node " << node.node_id 
                      << ": CPUs [";
            for (size_t i = 0; i < node.cpu_cores.size(); i++) {
                std::cout << node.cpu_cores[i];
                if (i < node.cpu_cores.size() - 1) std::cout << ", ";
            }
            std::cout << "], Memory Latency: " << node.memory_latency << "ns\n";
        }
    }
};

int main() {
    try {
        std::cout << "=== MULTI-PROCESSOR SCHEDULING DEMO ===\n\n";
        
        const int NUM_CORES = 4;
        MultiProcessorScheduler scheduler(NUM_CORES);
        
        // Start CPU schedulers
        std::vector<std::thread> cpu_threads;
        for (int i = 0; i < NUM_CORES; i++) {
            cpu_threads.emplace_back(&MultiProcessorScheduler::cpuScheduler, &scheduler, i);
        }
        
        // Start load balancer
        std::thread load_balancer_thread(&MultiProcessorScheduler::loadBalancer, &scheduler);
        
        // Generate tasks with different affinities
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> burst_dist(50, 200);
        std::uniform_int_distribution<> affinity_dist(0, NUM_CORES - 1);
        
        std::cout << "Generating tasks...\n";
        for (int i = 1; i <= 12; i++) {
            int burst_time = burst_dist(gen);
            int preferred_cpu = (i % 3 == 0) ? affinity_dist(gen) : -1; // Some tasks have affinity
            
            Task task(i, burst_time, preferred_cpu);
            scheduler.addTask(task);
            
            if (preferred_cpu >= 0) {
                std::cout << "Added Task " << i << " with CPU affinity to Core " << preferred_cpu << "\n";
            } else {
                std::cout << "Added Task " << i << " without CPU affinity\n";
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        // Wait for all tasks to complete
        scheduler.waitForCompletion();
        
        scheduler.displayStats();
        
        // Demonstrate NUMA awareness
        NUMAScheduler numa_scheduler;
        numa_scheduler.displayNUMATopology();
        
        std::cout << "\nOptimal core for NUMA node 0: " << numa_scheduler.selectOptimalCore(0) << "\n";
        std::cout << "Optimal core for NUMA node 1: " << numa_scheduler.selectOptimalCore(1) << "\n";
        
        // Stop scheduler
        scheduler.stop();
        
        if (load_balancer_thread.joinable()) {
            load_balancer_thread.join();
        }
        
        for (auto& thread : cpu_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        std::cout << "\nMulti-processor scheduling demo completed!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}