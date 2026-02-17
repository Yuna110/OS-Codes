/*
 * Exercise 4: Working Set Simulator
 * Calculates and tracks the working set of a process
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

class WorkingSetSimulator {
private:
    int windowSize;                     // Working set window size (Î”)
    vector<int> referenceString;        // Complete reference string
    vector<set<int>> workingSets;       // Working set at each time
    vector<int> workingSetSizes;        // Size of working set at each time
    
public:
    // Constructor
    WorkingSetSimulator(int delta) : windowSize(delta) {}
    
    // Calculate working set at a specific time
    set<int> calculateWorkingSet(int currentTime) {
        set<int> ws;
        
        // Look back at most windowSize references
        int startTime = max(0, currentTime - windowSize);
        
        for (int i = startTime; i <= currentTime; i++) {
            ws.insert(referenceString[i]);
        }
        
        return ws;
    }
    
    // Process reference string
    void processReferenceString(const vector<int>& refString) {
        referenceString = refString;
        workingSets.clear();
        workingSetSizes.clear();
        
        cout << "\n=== Working Set Analysis ===" << endl;
        cout << "Window Size (Î”): " << windowSize << endl;
        cout << "Reference String: ";
        for (int page : refString) {
            cout << page << " ";
        }
        cout << endl;
        cout << string(80, '-') << endl;
        
        // Calculate working set at each time
        for (int t = 0; t < refString.size(); t++) {
            set<int> ws = calculateWorkingSet(t);
            workingSets.push_back(ws);
            workingSetSizes.push_back(ws.size());
        }
    }
    
    // Display working sets
    void displayWorkingSets() {
        cout << "\n";
        cout << left << setw(6) << "Time" 
             << setw(6) << "Page" 
             << setw(30) << "Working Set" 
             << setw(8) << "WS Size" << endl;
        cout << string(80, '-') << endl;
        
        for (int t = 0; t < referenceString.size(); t++) {
            cout << left << setw(6) << (t + 1)
                 << setw(6) << referenceString[t]
                 << "{";
            
            bool first = true;
            for (int page : workingSets[t]) {
                if (!first) cout << ",";
                cout << page;
                first = false;
            }
            cout << "}";
            
            // Pad for alignment
            int padding = 28 - (workingSets[t].size() * 2);
            cout << string(max(0, padding), ' ');
            
            cout << setw(8) << workingSetSizes[t] << endl;
        }
    }
    
    // Display statistics
    void displayStatistics() {
        if (workingSetSizes.empty()) return;
        
        // Calculate average working set size
        double sum = 0;
        for (int size : workingSetSizes) {
            sum += size;
        }
        double avgWSS = sum / workingSetSizes.size();
        
        // Find min and max
        int minWSS = *min_element(workingSetSizes.begin(), workingSetSizes.end());
        int maxWSS = *max_element(workingSetSizes.begin(), workingSetSizes.end());
        
        cout << "\n=== Working Set Statistics ===" << endl;
        cout << "Average Working Set Size: " << fixed << setprecision(2) << avgWSS << endl;
        cout << "Minimum Working Set Size: " << minWSS << endl;
        cout << "Maximum Working Set Size: " << maxWSS << endl;
        
        // Show working set size distribution
        cout << "\nWorking Set Size Distribution:" << endl;
        for (int size = minWSS; size <= maxWSS; size++) {
            int count = 0;
            for (int ws : workingSetSizes) {
                if (ws == size) count++;
            }
            
            double percentage = (count * 100.0) / workingSetSizes.size();
            cout << "Size " << size << ": " << count << " times (" 
                 << fixed << setprecision(1) << percentage << "%)" << endl;
        }
    }
    
    // Visualize working set size over time
    void visualizeWorkingSetSize() {
        cout << "\n=== Working Set Size Visualization ===" << endl;
        
        int maxSize = *max_element(workingSetSizes.begin(), workingSetSizes.end());
        
        for (int t = 0; t < workingSetSizes.size(); t++) {
            cout << right << setw(3) << (t + 1) << " | ";
            for (int i = 0; i < workingSetSizes[t]; i++) {
                cout << "â–ˆ";
            }
            cout << " (" << workingSetSizes[t] << ")" << endl;
        }
    }
    
    // Estimate memory requirements
    void estimateMemoryRequirements(int pageSize) {
        if (workingSetSizes.empty()) return;
        
        double avgWSS = 0;
        for (int size : workingSetSizes) {
            avgWSS += size;
        }
        avgWSS /= workingSetSizes.size();
        
        int maxWSS = *max_element(workingSetSizes.begin(), workingSetSizes.end());
        
        cout << "\n=== Memory Requirements (Page Size = " << pageSize << " KB) ===" << endl;
        cout << "Average Memory Need: " << fixed << setprecision(2) 
             << (avgWSS * pageSize) << " KB" << endl;
        cout << "Peak Memory Need: " << (maxWSS * pageSize) << " KB" << endl;
    }
};

int main() {
    cout << "=== Working Set Simulator ===" << endl;
    
    // Test case 1: Small window
    vector<int> refString1 = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
    
    cout << "\n*** Test 1: Window Size = 5 ***" << endl;
    WorkingSetSimulator wss1(5);
    wss1.processReferenceString(refString1);
    wss1.displayWorkingSets();
    wss1.displayStatistics();
    wss1.visualizeWorkingSetSize();
    wss1.estimateMemoryRequirements(4);  // 4 KB pages
    
    // Test case 2: Larger window
    cout << "\n\n*** Test 2: Window Size = 3 ***" << endl;
    WorkingSetSimulator wss2(3);
    wss2.processReferenceString(refString1);
    wss2.displayWorkingSets();
    wss2.displayStatistics();
    wss2.visualizeWorkingSetSize();
    
    // Test case 3: Very small window
    cout << "\n\n*** Test 3: Window Size = 2 ***" << endl;
    WorkingSetSimulator wss3(2);
    wss3.processReferenceString(refString1);
    wss3.displayWorkingSets();
    wss3.displayStatistics();
    
    // Test case 4: Compare different window sizes
    cout << "\n\n=== Comparison of Different Window Sizes ===" << endl;
    vector<int> refString2 = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    
    cout << "\n" << left << setw(15) << "Window Size" 
         << setw(20) << "Avg WS Size" 
         << setw(20) << "Max WS Size" << endl;
    cout << string(55, '-') << endl;
    
    for (int delta = 2; delta <= 6; delta++) {
        WorkingSetSimulator wss(delta);
        wss.processReferenceString(refString2);
        
        // Calculate statistics without displaying
        double sum = 0;
        int maxSize = 0;
        for (int t = 0; t < refString2.size(); t++) {
            set<int> ws = wss.calculateWorkingSet(t);
            sum += ws.size();
            maxSize = max(maxSize, (int)ws.size());
        }
        double avgSize = sum / refString2.size();
        
        cout << left << setw(15) << delta 
             << setw(20) << fixed << setprecision(2) << avgSize
             << setw(20) << maxSize << endl;
    }
    
    return 0;
}
