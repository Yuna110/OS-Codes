/*
 * Exercise 3: LRU Page Replacement Algorithm
 * Implements Least Recently Used page replacement using timestamps
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <algorithm>
#include <limits>

using namespace std;

class LRUPageReplacement {
private:
    int numFrames;                      // Number of physical frames
    vector<int> frames;                 // Current pages in frames
    map<int, int> lastAccessTime;       // Last access time for each page
    int currentTime;                    // Current timestamp
    int pageFaults;                     // Count of page faults
    int totalReferences;                // Total page references
    
    // Check if page is in memory
    bool isPageInMemory(int page) {
        return find(frames.begin(), frames.end(), page) != frames.end();
    }
    
    // Find empty frame
    int findEmptyFrame() {
        for (int i = 0; i < numFrames; i++) {
            if (frames[i] == -1) {
                return i;
            }
        }
        return -1;
    }
    
    // Find least recently used page
    int findLRUPage() {
        int lruPage = -1;
        int minTime = numeric_limits<int>::max();
        
        for (int page : frames) {
            if (page != -1 && lastAccessTime[page] < minTime) {
                minTime = lastAccessTime[page];
                lruPage = page;
            }
        }
        
        return lruPage;
    }
    
public:
    // Constructor
    LRUPageReplacement(int frames) : numFrames(frames), currentTime(0), 
                                     pageFaults(0), totalReferences(0) {
        this->frames.resize(frames, -1);
    }
    
    // Process a page reference
    void referencePage(int page) {
        totalReferences++;
        currentTime++;
        
        cout << "\nReference: " << page << " (Time: " << currentTime << ") | ";
        
        // Update last access time
        lastAccessTime[page] = currentTime;
        
        // Check if page is already in memory
        if (isPageInMemory(page)) {
            cout << "HIT";
        } else {
            cout << "FAULT";
            pageFaults++;
            
            // Check for empty frame
            int emptyFrame = findEmptyFrame();
            
            if (emptyFrame != -1) {
                // Empty frame available
                frames[emptyFrame] = page;
            } else {
                // No empty frame - replace LRU page
                int lruPage = findLRUPage();
                
                // Find and replace LRU page
                for (int i = 0; i < numFrames; i++) {
                    if (frames[i] == lruPage) {
                        frames[i] = page;
                        break;
                    }
                }
                
                cout << " (Replaced: " << lruPage << ")";
            }
        }
        
        // Display current frame state
        cout << " | Frames: [";
        for (int i = 0; i < numFrames; i++) {
            if (frames[i] == -1) {
                cout << " -";
            } else {
                cout << setw(2) << frames[i];
            }
            if (i < numFrames - 1) cout << " ";
        }
        cout << "]" << endl;
    }
    
    // Process reference string
    void processReferenceString(const vector<int>& refString) {
        cout << "\n=== Processing Reference String ===" << endl;
        cout << "Number of Frames: " << numFrames << endl;
        cout << "Reference String: ";
        for (int page : refString) {
            cout << page << " ";
        }
        cout << endl;
        cout << string(70, '-') << endl;
        
        for (int page : refString) {
            referencePage(page);
        }
    }
    
    // Display statistics
    void displayStatistics() {
        cout << "\n=== Statistics ===" << endl;
        cout << "Total References: " << totalReferences << endl;
        cout << "Page Faults: " << pageFaults << endl;
        cout << "Page Hits: " << (totalReferences - pageFaults) << endl;
        cout << "Page Fault Rate: " << fixed << setprecision(2) 
             << (pageFaults * 100.0 / totalReferences) << "%" << endl;
        cout << "Page Hit Rate: " << fixed << setprecision(2) 
             << ((totalReferences - pageFaults) * 100.0 / totalReferences) << "%" << endl;
    }
    
    int getPageFaults() const { return pageFaults; }
    
    // Reset for new test
    void reset() {
        frames.assign(numFrames, -1);
        lastAccessTime.clear();
        currentTime = 0;
        pageFaults = 0;
        totalReferences = 0;
    }
};

// FIFO for comparison
class FIFOComparison {
private:
    int numFrames;
    vector<int> frames;
    vector<int> insertOrder;
    int pageFaults;
    
    bool isPageInMemory(int page) {
        return find(frames.begin(), frames.end(), page) != frames.end();
    }
    
    int findEmptyFrame() {
        for (int i = 0; i < numFrames; i++) {
            if (frames[i] == -1) return i;
        }
        return -1;
    }
    
public:
    FIFOComparison(int frames) : numFrames(frames), pageFaults(0) {
        this->frames.resize(frames, -1);
    }
    
    void referencePage(int page) {
        if (!isPageInMemory(page)) {
            pageFaults++;
            int emptyFrame = findEmptyFrame();
            
            if (emptyFrame != -1) {
                frames[emptyFrame] = page;
                insertOrder.push_back(page);
            } else {
                int victimPage = insertOrder.front();
                insertOrder.erase(insertOrder.begin());
                
                for (int i = 0; i < numFrames; i++) {
                    if (frames[i] == victimPage) {
                        frames[i] = page;
                        break;
                    }
                }
                insertOrder.push_back(page);
            }
        }
    }
    
    void processReferenceString(const vector<int>& refString) {
        for (int page : refString) {
            referencePage(page);
        }
    }
    
    int getPageFaults() const { return pageFaults; }
};

int main() {
    cout << "=== LRU Page Replacement Algorithm ===" << endl;
    
    // Test case 1: Same as FIFO test
    vector<int> refString1 = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2};
    LRUPageReplacement lru1(3);
    lru1.processReferenceString(refString1);
    lru1.displayStatistics();
    
    // Test case 2: Comparison with FIFO
    cout << "\n\n=== Comparison: LRU vs FIFO ===" << endl;
    vector<int> refString2 = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    
    cout << "\n--- LRU Algorithm ---" << endl;
    LRUPageReplacement lru2(4);
    lru2.processReferenceString(refString2);
    lru2.displayStatistics();
    
    cout << "\n--- FIFO Algorithm (for comparison) ---" << endl;
    FIFOComparison fifo(4);
    fifo.processReferenceString(refString2);
    cout << "FIFO Page Faults: " << fifo.getPageFaults() << endl;
    
    // Comparison table
    cout << "\n=== Comparison Table ===" << endl;
    cout << string(50, '-') << endl;
    cout << left << setw(20) << "Algorithm" << setw(15) << "Page Faults" 
         << setw(15) << "Difference" << endl;
    cout << string(50, '-') << endl;
    cout << left << setw(20) << "LRU" << setw(15) << lru2.getPageFaults() 
         << setw(15) << "-" << endl;
    cout << left << setw(20) << "FIFO" << setw(15) << fifo.getPageFaults() 
         << setw(15) << (fifo.getPageFaults() - lru2.getPageFaults()) << endl;
    cout << string(50, '-') << endl;
    
    if (lru2.getPageFaults() < fifo.getPageFaults()) {
        cout << "LRU performs better (fewer page faults)" << endl;
    } else if (lru2.getPageFaults() > fifo.getPageFaults()) {
        cout << "FIFO performs better (fewer page faults)" << endl;
    } else {
        cout << "Both algorithms have the same performance" << endl;
    }
    
    // Test case 3: Another comparison
    cout << "\n\n=== Another Test Case ===" << endl;
    vector<int> refString3 = {0, 1, 2, 3, 0, 1, 4, 0, 1, 2, 3, 4};
    
    LRUPageReplacement lru3(3);
    lru3.processReferenceString(refString3);
    lru3.displayStatistics();
    
    return 0;
}
