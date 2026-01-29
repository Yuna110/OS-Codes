#include <iostream>
#include <vector>
#include <algorithm>

class BankersAlgorithm {
private:
    int numProcesses;
    int numResources;
    
    std::vector<std::vector<int>> allocation;   // Currently allocated
    std::vector<std::vector<int>> maximum;      // Maximum demand
    std::vector<int> available;                  // Available resources
    
public:
    BankersAlgorithm(int processes, int resources) 
        : numProcesses(processes), numResources(resources) {
        allocation.resize(processes, std::vector<int>(resources, 0));
        maximum.resize(processes, std::vector<int>(resources, 0));
        available.resize(resources, 0);
    }
    
    void setAvailable(const std::vector<int>& avail) {
        available = avail;
    }
    
    void setMaximum(int process, const std::vector<int>& max) {
        maximum[process] = max;
    }
    
    void setAllocation(int process, const std::vector<int>& alloc) {
        allocation[process] = alloc;
    }
    
    // Calculate Need matrix (Maximum - Allocation)
    std::vector<std::vector<int>> calculateNeed() {
        std::vector<std::vector<int>> need(numProcesses, 
                                           std::vector<int>(numResources));
        for (int i = 0; i < numProcesses; i++) {
            for (int j = 0; j < numResources; j++) {
                need[i][j] = maximum[i][j] - allocation[i][j];
            }
        }
        return need;
    }
    
    // Check if system is in safe state
    bool isSafeState(std::vector<int>& safeSequence) {
        std::vector<int> work = available;
        std::vector<bool> finish(numProcesses, false);
        std::vector<std::vector<int>> need = calculateNeed();
        
        safeSequence.clear();
        
        // Try to find safe sequence
        for (int count = 0; count < numProcesses; count++) {
            bool found = false;
            
            for (int i = 0; i < numProcesses; i++) {
                if (!finish[i]) {
                    // Check if need[i] <= work
                    bool canAllocate = true;
                    for (int j = 0; j < numResources; j++) {
                        if (need[i][j] > work[j]) {
                            canAllocate = false;
                            break;
                        }
                    }
                    
                    if (canAllocate) {
                        // Allocate resources
                        for (int j = 0; j < numResources; j++) {
                            work[j] += allocation[i][j];
                        }
                        
                        safeSequence.push_back(i);
                        finish[i] = true;
                        found = true;
                    }
                }
            }
            
            if (!found) {
                return false; // No safe sequence exists
            }
        }
        
        return true; // Safe sequence found
    }
    
    // Request resources for a process
    bool requestResources(int process, const std::vector<int>& request) {
        std::vector<std::vector<int>> need = calculateNeed();
        
        // Check if request <= need
        for (int i = 0; i < numResources; i++) {
            if (request[i] > need[process][i]) {
                std::cout << "Error: Process exceeded maximum claim\n";
                return false;
            }
        }
        
        // Check if request <= available
        for (int i = 0; i < numResources; i++) {
            if (request[i] > available[i]) {
                std::cout << "Process must wait - insufficient resources\n";
                return false;
            }
        }
        
        // Pretend to allocate
        for (int i = 0; i < numResources; i++) {
            available[i] -= request[i];
            allocation[process][i] += request[i];
        }
        
        // Check if safe
        std::vector<int> safeSeq;
        if (isSafeState(safeSeq)) {
            std::cout << "Request granted! Safe sequence: ";
            for (int p : safeSeq) {
                std::cout << "P" << p << " ";
            }
            std::cout << "\n";
            return true;
        } else {
            // Rollback
            for (int i = 0; i < numResources; i++) {
                available[i] += request[i];
                allocation[process][i] -= request[i];
            }
            std::cout << "Request denied - would lead to unsafe state\n";
            return false;
        }
    }
    
    void printState() {
        std::cout << "\n=== Current State ===\n";
        
        std::cout << "Available: ";
        for (int val : available) {
            std::cout << val << " ";
        }
        std::cout << "\n\nAllocation Matrix:\n";
        for (int i = 0; i < numProcesses; i++) {
            std::cout << "P" << i << ": ";
            for (int val : allocation[i]) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }
        
        std::cout << "\nMaximum Matrix:\n";
        for (int i = 0; i < numProcesses; i++) {
            std::cout << "P" << i << ": ";
            for (int val : maximum[i]) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }
        
        auto need = calculateNeed();
        std::cout << "\nNeed Matrix:\n";
        for (int i = 0; i < numProcesses; i++) {
            std::cout << "P" << i << ": ";
            for (int val : need[i]) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }
    }
};

int main() {
    // Example: 5 processes, 3 resource types (A, B, C)
    BankersAlgorithm banker(5, 3);
    
    // Available resources: A=3, B=3, C=2
    banker.setAvailable({3, 3, 2});
    
    // Set Maximum matrix
    banker.setMaximum(0, {7, 5, 3});
    banker.setMaximum(1, {3, 2, 2});
    banker.setMaximum(2, {9, 0, 2});
    banker.setMaximum(3, {2, 2, 2});
    banker.setMaximum(4, {4, 3, 3});
    
    // Set Allocation matrix
    banker.setAllocation(0, {0, 1, 0});
    banker.setAllocation(1, {2, 0, 0});
    banker.setAllocation(2, {3, 0, 2});
    banker.setAllocation(3, {2, 1, 1});
    banker.setAllocation(4, {0, 0, 2});
    
    banker.printState();
    
    // Check if initial state is safe
    std::vector<int> safeSeq;
    if (banker.isSafeState(safeSeq)) {
        std::cout << "\nSystem is in SAFE state\n";
        std::cout << "Safe sequence: ";
        for (int p : safeSeq) {
            std::cout << "P" << p << " ";
        }
        std::cout << "\n";
    }
    
    // Process 1 requests (1, 0, 2)
    std::cout << "\n--- P1 requests (1, 0, 2) ---\n";
    banker.requestResources(1, {1, 0, 2});
    
    // Process 4 requests (3, 3, 0)
    std::cout << "\n--- P4 requests (3, 3, 0) ---\n";
    banker.requestResources(4, {3, 3, 0});
    
    // Process 0 requests (0, 2, 0)
    std::cout << "\n--- P0 requests (0, 2, 0) ---\n";
    banker.requestResources(0, {0, 2, 0});
    
    return 0;
}