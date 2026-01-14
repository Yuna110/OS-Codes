// File: scheduling_metrics.cpp
// Compile: g++ -o scheduling_metrics scheduling_metrics.cpp -std=c++17

#include <iostream>
#include <vector>
#include <algorithm>
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
          remaining_time(bt), priority(pr) {}
};

class MetricsCalculator {
private:
    std::vector<Process> processes;
    int total_time;
    int cpu_idle_time;

public:
    void setProcesses(const std::vector<Process>& procs) {
        processes = procs;
        calculateTotalTime();
    }
    
    void calculateTotalTime() {
        if (processes.empty()) return;
        
        int max_completion = 0;
        for (const auto& p : processes) {
            max_completion = std::max(max_completion, p.completion_time);
        }
        total_time = max_completion;
    }
    
    double getCPUUtilization() {
        int cpu_busy_time = total_time - cpu_idle_time;
        return (static_cast<double>(cpu_busy_time) / total_time) * 100.0;
    }
    
    double getThroughput() {
        return static_cast<double>(processes.size()) / total_time;
    }
    
    double getAverageWaitingTime() {
        int total_waiting = 0;
        for (const auto& p : processes) {
            total_waiting += p.waiting_time;
        }
        return static_cast<double>(total_waiting) / processes.size();
    }
    
    double getAverageTurnaroundTime() {
        int total_turnaround = 0;
        for (const auto& p : processes) {
            total_turnaround += p.turnaround_time;
        }
        return static_cast<double>(total_turnaround) / processes.size();
    }
    
    double getAverageResponseTime() {
        // Assuming response time equals waiting time for simplicity
        return getAverageWaitingTime();
    }
    
    void displayMetrics() {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "\n=== SCHEDULING METRICS ===\n";
        std::cout << "CPU Utilization: " << getCPUUtilization() << "%\n";
        std::cout << "Throughput: " << getThroughput() << " processes/unit time\n";
        std::cout << "Average Waiting Time: " << getAverageWaitingTime() << " units\n";
        std::cout << "Average Turnaround Time: " << getAverageTurnaroundTime() << " units\n";
        std::cout << "Average Response Time: " << getAverageResponseTime() << " units\n";
    }
    
    void setCPUIdleTime(int idle) { cpu_idle_time = idle; }
};

// Demo usage
int main() {
    std::vector<Process> sample_processes = {
        Process(1, 0, 7), Process(2, 2, 4), Process(3, 4, 1)
    };
    
    // Simulate some completion times
    sample_processes[0].completion_time = 7;
    sample_processes[0].turnaround_time = 7;
    sample_processes[0].waiting_time = 0;
    
    sample_processes[1].completion_time = 11;
    sample_processes[1].turnaround_time = 9;
    sample_processes[1].waiting_time = 5;
    
    sample_processes[2].completion_time = 12;
    sample_processes[2].turnaround_time = 8;
    sample_processes[2].waiting_time = 7;
    
    MetricsCalculator calc;
    calc.setProcesses(sample_processes);
    calc.setCPUIdleTime(0);
    calc.displayMetrics();
    
    return 0;
}