// File: process_basics.cpp
// Compile: g++ -o process_basics process_basics.cpp -std=c++17

#include <iostream>
#include <vector>
#include <queue>
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

class ProcessScheduler {
public:
    std::vector<Process> processes;
    
    void addProcess(int pid, int arrival, int burst, int priority = 0) {
        processes.emplace_back(pid, arrival, burst, priority);
    }
    
    void displayProcesses() {
        std::cout << std::setw(5) << "PID" << std::setw(10) << "Arrival" 
                  << std::setw(10) << "Burst" << std::setw(12) << "Completion" 
                  << std::setw(12) << "Turnaround" << std::setw(10) << "Waiting\n";
        std::cout << std::string(60, '-') << "\n";
        
        for (const auto& p : processes) {
            std::cout << std::setw(5) << p.pid << std::setw(10) << p.arrival_time
                      << std::setw(10) << p.burst_time << std::setw(12) << p.completion_time
                      << std::setw(12) << p.turnaround_time << std::setw(10) << p.waiting_time << "\n";
        }
    }
    
    double calculateAverageWaitingTime() {
        int total = 0;
        for (const auto& p : processes) {
            total += p.waiting_time;
        }
        return static_cast<double>(total) / processes.size();
    }
    
    double calculateAverageTurnaroundTime() {
        int total = 0;
        for (const auto& p : processes) {
            total += p.turnaround_time;
        }
        return static_cast<double>(total) / processes.size();
    }
};

// Demo main function
int main() {
    ProcessScheduler scheduler;
    
    // Example processes
    scheduler.addProcess(1, 0, 7);
    scheduler.addProcess(2, 2, 4);
    scheduler.addProcess(3, 4, 1);
    scheduler.addProcess(4, 5, 4);
    
    std::cout << "Basic Process Structure Demo\n";
    std::cout << "============================\n";
    scheduler.displayProcesses();
    
    return 0;
}