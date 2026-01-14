// File: scheduling_algorithms.cpp
// Compile: g++ -o scheduling_algorithms scheduling_algorithms.cpp -std=c++17

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>
#include <iomanip>

struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int priority;
    
    Process(int id, int at, int bt, int pr = 0) 
        : pid(id), arrival_time(at), burst_time(bt), 
          remaining_time(bt), priority(pr), 
          completion_time(0), turnaround_time(0), waiting_time(0) {}
};

// ProcessScheduler class to handle display and calculations
class ProcessScheduler {
public:
    std::vector<Process> processes;
    
    void displayProcesses() {
        std::cout << std::setw(5) << "PID" 
                  << std::setw(10) << "Arrival" 
                  << std::setw(10) << "Burst" 
                  << std::setw(12) << "Completion" 
                  << std::setw(12) << "Turnaround" 
                  << std::setw(10) << "Waiting" << "\n";
        std::cout << std::string(60, '-') << "\n";
        
        for (const auto& p : processes) {
            std::cout << std::setw(5) << p.pid
                      << std::setw(10) << p.arrival_time
                      << std::setw(10) << p.burst_time
                      << std::setw(12) << p.completion_time
                      << std::setw(12) << p.turnaround_time
                      << std::setw(10) << p.waiting_time << "\n";
        }
    }
    
    double calculateAverageWaitingTime() {
        if (processes.empty()) return 0.0;
        
        double total = 0;
        for (const auto& p : processes) {
            total += p.waiting_time;
        }
        return total / processes.size();
    }
    
    double calculateAverageTurnaroundTime() {
        if (processes.empty()) return 0.0;
        
        double total = 0;
        for (const auto& p : processes) {
            total += p.turnaround_time;
        }
        return total / processes.size();
    }
};

class SchedulingAlgorithms {
public:
    // FCFS Scheduling
    static void FCFS(std::vector<Process>& processes) {
        std::sort(processes.begin(), processes.end(), 
                  [](const Process& a, const Process& b) {
                      return a.arrival_time < b.arrival_time;
                  });
        
        int current_time = 0;
        for (auto& p : processes) {
            if (current_time < p.arrival_time) {
                current_time = p.arrival_time;
            }
            p.completion_time = current_time + p.burst_time;
            p.turnaround_time = p.completion_time - p.arrival_time;
            p.waiting_time = p.turnaround_time - p.burst_time;
            current_time = p.completion_time;
        }
    }
    
    // SJF Non-preemptive Scheduling
    static void SJF(std::vector<Process>& processes) {
        int n = processes.size();
        std::vector<bool> completed(n, false);
        int current_time = 0;
        int completed_count = 0;
        
        while (completed_count < n) {
            int shortest_job = -1;
            int min_burst = INT_MAX;
            
            for (int i = 0; i < n; i++) {
                if (!completed[i] && processes[i].arrival_time <= current_time) {
                    if (processes[i].burst_time < min_burst) {
                        min_burst = processes[i].burst_time;
                        shortest_job = i;
                    }
                }
            }
            
            if (shortest_job == -1) {
                current_time++;
                continue;
            }
            
            processes[shortest_job].completion_time = current_time + processes[shortest_job].burst_time;
            processes[shortest_job].turnaround_time = processes[shortest_job].completion_time - processes[shortest_job].arrival_time;
            processes[shortest_job].waiting_time = processes[shortest_job].turnaround_time - processes[shortest_job].burst_time;
            
            current_time = processes[shortest_job].completion_time;
            completed[shortest_job] = true;
            completed_count++;
        }
    }
    
    // SRTF (Preemptive SJF) Scheduling
    static void SRTF(std::vector<Process>& processes) {
        int n = processes.size();
        std::vector<int> remaining_time(n);
        
        for (int i = 0; i < n; i++) {
            remaining_time[i] = processes[i].burst_time;
        }
        
        int current_time = 0;
        int completed = 0;
        
        while (completed < n) {
            int shortest = -1;
            int min_remaining = INT_MAX;
            
            for (int i = 0; i < n; i++) {
                if (processes[i].arrival_time <= current_time && 
                    remaining_time[i] < min_remaining && remaining_time[i] > 0) {
                    min_remaining = remaining_time[i];
                    shortest = i;
                }
            }
            
            if (shortest == -1) {
                current_time++;
                continue;
            }
            
            remaining_time[shortest]--;
            current_time++;
            
            if (remaining_time[shortest] == 0) {
                completed++;
                processes[shortest].completion_time = current_time;
                processes[shortest].turnaround_time = processes[shortest].completion_time - processes[shortest].arrival_time;
                processes[shortest].waiting_time = processes[shortest].turnaround_time - processes[shortest].burst_time;
            }
        }
    }
    
    // Round Robin Scheduling
    static void RoundRobin(std::vector<Process>& processes, int quantum) {
        std::queue<int> ready_queue;
        std::vector<int> remaining_time(processes.size());
        std::vector<bool> in_queue(processes.size(), false);
        
        for (size_t i = 0; i < processes.size(); i++) {
            remaining_time[i] = processes[i].burst_time;
        }
        
        int current_time = 0;
        int completed = 0;
        
        // Sort by arrival time for initial processing
        std::vector<int> arrival_order(processes.size());
        for (size_t i = 0; i < processes.size(); i++) {
            arrival_order[i] = i;
        }
        std::sort(arrival_order.begin(), arrival_order.end(),
                  [&processes](int a, int b) {
                      return processes[a].arrival_time < processes[b].arrival_time;
                  });
        
        // Add first process if available
        if (!processes.empty() && processes[arrival_order[0]].arrival_time <= current_time) {
            ready_queue.push(arrival_order[0]);
            in_queue[arrival_order[0]] = true;
        }
        
        while (!ready_queue.empty() || completed < (int)processes.size()) {
            if (ready_queue.empty()) {
                // Find next arriving process
                for (int idx : arrival_order) {
                    if (remaining_time[idx] > 0 && processes[idx].arrival_time > current_time) {
                        current_time = processes[idx].arrival_time;
                        ready_queue.push(idx);
                        in_queue[idx] = true;
                        break;
                    }
                }
                if (ready_queue.empty()) break; // No more processes
            }
            
            int current_process = ready_queue.front();
            ready_queue.pop();
            in_queue[current_process] = false;
            
            int exec_time = std::min(quantum, remaining_time[current_process]);
            remaining_time[current_process] -= exec_time;
            current_time += exec_time;
            
            // Add newly arrived processes
            for (int idx : arrival_order) {
                if (!in_queue[idx] && remaining_time[idx] > 0 && 
                    processes[idx].arrival_time <= current_time && idx != current_process) {
                    ready_queue.push(idx);
                    in_queue[idx] = true;
                }
            }
            
            if (remaining_time[current_process] == 0) {
                completed++;
                processes[current_process].completion_time = current_time;
                processes[current_process].turnaround_time = 
                    processes[current_process].completion_time - processes[current_process].arrival_time;
                processes[current_process].waiting_time = 
                    processes[current_process].turnaround_time - processes[current_process].burst_time;
            } else {
                ready_queue.push(current_process);
                in_queue[current_process] = true;
            }
        }
    }
    
    // Priority Scheduling (Non-preemptive)
    static void PriorityScheduling(std::vector<Process>& processes) {
        int n = processes.size();
        std::vector<bool> completed(n, false);
        int current_time = 0;
        int completed_count = 0;
        
        while (completed_count < n) {
            int highest_priority_job = -1;
            int highest_priority = INT_MAX; // Lower number = higher priority
            
            for (int i = 0; i < n; i++) {
                if (!completed[i] && processes[i].arrival_time <= current_time) {
                    if (processes[i].priority < highest_priority) {
                        highest_priority = processes[i].priority;
                        highest_priority_job = i;
                    }
                }
            }
            
            if (highest_priority_job == -1) {
                current_time++;
                continue;
            }
            
            processes[highest_priority_job].completion_time = current_time + processes[highest_priority_job].burst_time;
            processes[highest_priority_job].turnaround_time = 
                processes[highest_priority_job].completion_time - processes[highest_priority_job].arrival_time;
            processes[highest_priority_job].waiting_time = 
                processes[highest_priority_job].turnaround_time - processes[highest_priority_job].burst_time;
            
            current_time = processes[highest_priority_job].completion_time;
            completed[highest_priority_job] = true;
            completed_count++;
        }
    }
};

// Demo main function
int main() {
    // Test data pid, arrival_time, burst_time, priority
    std::vector<Process> processes = {
        Process(1, 0, 7, 2),
        Process(2, 2, 4, 1),
        Process(3, 4, 1, 4),
        Process(4, 5, 4, 3)
    };
    
    std::cout << "=== FCFS Scheduling ===\n";
    auto fcfs_processes = processes;
    SchedulingAlgorithms::FCFS(fcfs_processes);
    ProcessScheduler scheduler;
    scheduler.processes = fcfs_processes;
    scheduler.displayProcesses();
    std::cout << "Average Waiting Time: " << scheduler.calculateAverageWaitingTime() << "\n";
    std::cout << "Average Turnaround Time: " << scheduler.calculateAverageTurnaroundTime() << "\n\n";
    
    std::cout << "=== SJF Scheduling ===\n";
    auto sjf_processes = processes;
    SchedulingAlgorithms::SJF(sjf_processes);
    scheduler.processes = sjf_processes;
    scheduler.displayProcesses();
    std::cout << "Average Waiting Time: " << scheduler.calculateAverageWaitingTime() << "\n";
    std::cout << "Average Turnaround Time: " << scheduler.calculateAverageTurnaroundTime() << "\n\n";
    
    std::cout << "=== SRTF Scheduling ===\n";
    auto srtf_processes = processes;
    SchedulingAlgorithms::SRTF(srtf_processes);
    scheduler.processes = srtf_processes;
    scheduler.displayProcesses();
    std::cout << "Average Waiting Time: " << scheduler.calculateAverageWaitingTime() << "\n";
    std::cout << "Average Turnaround Time: " << scheduler.calculateAverageTurnaroundTime() << "\n\n";
    
    std::cout << "=== Round Robin (Quantum=2) Scheduling ===\n";
    auto rr_processes = processes;
    SchedulingAlgorithms::RoundRobin(rr_processes, 2);
    scheduler.processes = rr_processes;
    scheduler.displayProcesses();
    std::cout << "Average Waiting Time: " << scheduler.calculateAverageWaitingTime() << "\n";
    std::cout << "Average Turnaround Time: " << scheduler.calculateAverageTurnaroundTime() << "\n\n";
    
    std::cout << "=== Priority Scheduling ===\n";
    auto priority_processes = processes;
    SchedulingAlgorithms::PriorityScheduling(priority_processes);
    scheduler.processes = priority_processes;
    scheduler.displayProcesses();
    std::cout << "Average Waiting Time: " << scheduler.calculateAverageWaitingTime() << "\n";
    std::cout << "Average Turnaround Time: " << scheduler.calculateAverageTurnaroundTime() << "\n\n";
    
    return 0;
}