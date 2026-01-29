#include <iostream>
#include <vector>
#include <queue>

class RAGDetector {
private:
    int numProcesses;
    int numResources;
    
    // Allocation[i][j] = process i holds j instances of resource
    std::vector<std::vector<int>> allocation;
    // Request[i][j] = process i requests j instances of resource
    std::vector<std::vector<int>> request;
    // Available[j] = available instances of resource j
    std::vector<int> available;
    
public:
    RAGDetector(int processes, int resources) 
        : numProcesses(processes), numResources(resources) {
        allocation.resize(processes, std::vector<int>(resources, 0));
        request.resize(processes, std::vector<int>(resources, 0));
        available.resize(resources, 0);
    }
    
    void setAllocation(int process, int resource, int count) {
        allocation[process][resource] = count;
    }
    
    void setRequest(int process, int resource, int count) {
        request[process][resource] = count;
    }
    
    void setAvailable(int resource, int count) {
        available[resource] = count;
    }
    
    // Detect deadlock using resource allocation
    bool detectDeadlock(std::vector<int>& deadlockedProcesses) {
        std::vector<int> work = available;
        std::vector<bool> finish(numProcesses, false);
        
        // Mark processes with no requests as finished
        for (int i = 0; i < numProcesses; i++) {
            bool hasRequest = false;
            for (int j = 0; j < numResources; j++) {
                if (request[i][j] > 0) {
                    hasRequest = true;
                    break;
                }
            }
            if (!hasRequest) {
                finish[i] = true;
            }
        }
        
        // Try to satisfy requests
        bool progress = true;
        while (progress) {
            progress = false;
            
            for (int i = 0; i < numProcesses; i++) {
                if (!finish[i]) {
                    // Check if request can be satisfied
                    bool canSatisfy = true;
                    for (int j = 0; j < numResources; j++) {
                        if (request[i][j] > work[j]) {
                            canSatisfy = false;
                            break;
                        }
                    }
                    
                    if (canSatisfy) {
                        // Grant resources
                        for (int j = 0; j < numResources; j++) {
                            work[j] += allocation[i][j];
                        }
                        finish[i] = true;
                        progress = true;
                    }
                }
            }
        }
        
        // Collect deadlocked processes
        for (int i = 0; i < numProcesses; i++) {
            if (!finish[i]) {
                deadlockedProcesses.push_back(i);
            }
        }
        
        return !deadlockedProcesses.empty();
    }
    
    void printState() {
        std::cout << "\n=== Resource Allocation State ===\n";
        
        std::cout << "Available: ";
        for (int i = 0; i < numResources; i++) {
            std::cout << "R" << i << "=" << available[i] << " ";
        }
        std::cout << "\n\nAllocation:\n";
        for (int i = 0; i < numProcesses; i++) {
            std::cout << "P" << i << ": ";
            for (int j = 0; j < numResources; j++) {
                std::cout << allocation[i][j] << " ";
            }
            std::cout << "\n";
        }
        
        std::cout << "\nRequest:\n";
        for (int i = 0; i < numProcesses; i++) {
            std::cout << "P" << i << ": ";
            for (int j = 0; j < numResources; j++) {
                std::cout << request[i][j] << " ";
            }
            std::cout << "\n";
        }
    }
};

int main() {
    // 5 processes, 3 resource types
    RAGDetector detector(5, 3);
    
    // Set available resources
    detector.setAvailable(0, 0); // R0: 0 available
    detector.setAvailable(1, 0); // R1: 0 available
    detector.setAvailable(2, 0); // R2: 0 available
    
    // Set allocations
    detector.setAllocation(0, 0, 1);
    detector.setAllocation(1, 1, 1);
    detector.setAllocation(2, 2, 1);
    detector.setAllocation(3, 0, 1);
    detector.setAllocation(4, 1, 1);
    
    // Set requests (creating deadlock)
    detector.setRequest(0, 1, 1); // P0 wants R1
    detector.setRequest(1, 2, 1); // P1 wants R2
    detector.setRequest(2, 0, 1); // P2 wants R0 (cycle!)
    detector.setRequest(3, 1, 1); // P3 wants R1
    detector.setRequest(4, 2, 1); // P4 wants R2
    
    detector.printState();
    
    // Detect deadlock
    std::vector<int> deadlocked;
    if (detector.detectDeadlock(deadlocked)) {
        std::cout << "\n🚨 DEADLOCK DETECTED!\n";
        std::cout << "Deadlocked processes: ";
        for (int p : deadlocked) {
            std::cout << "P" << p << " ";
        }
        std::cout << "\n";
    } else {
        std::cout << "\n✓ No deadlock detected\n";
    }
    
    return 0;
}