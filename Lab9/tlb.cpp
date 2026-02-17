/*
 * Exercise 5: TLB (Translation Lookaside Buffer) Simulator
 * Simulates a TLB with LRU replacement policy
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

// TLB Entry
struct TLBEntry {
    int pageNumber;
    int frameNumber;
    int lastAccessTime;
    bool valid;
    
    TLBEntry() : pageNumber(-1), frameNumber(-1), lastAccessTime(0), valid(false) {}
};

class TLBSimulator {
private:
    int tlbSize;                        // Number of TLB entries
    vector<TLBEntry> tlb;               // TLB entries
    map<int, int> pageTable;            // Full page table (page -> frame)
    int currentTime;                    // Current timestamp
    int tlbHits;                        // Count of TLB hits
    int tlbMisses;                      // Count of TLB misses
    int totalAccesses;                  // Total memory accesses
    
    // TLB access times (in nanoseconds)
    const int TLB_ACCESS_TIME = 20;
    const int MEMORY_ACCESS_TIME = 100;
    
    // Find page in TLB
    int findInTLB(int pageNumber) {
        for (int i = 0; i < tlbSize; i++) {
            if (tlb[i].valid && tlb[i].pageNumber == pageNumber) {
                return i;
            }
        }
        return -1;
    }
    
    // Find empty TLB entry
    int findEmptyTLBEntry() {
        for (int i = 0; i < tlbSize; i++) {
            if (!tlb[i].valid) {
                return i;
            }
        }
        return -1;
    }
    
    // Find LRU entry in TLB
    int findLRUEntry() {
        int lruIndex = 0;
        int minTime = tlb[0].lastAccessTime;
        
        for (int i = 1; i < tlbSize; i++) {
            if (tlb[i].lastAccessTime < minTime) {
                minTime = tlb[i].lastAccessTime;
                lruIndex = i;
            }
        }
        
        return lruIndex;
    }
    
    // Add entry to TLB
    void addToTLB(int pageNumber, int frameNumber) {
        int index = findEmptyTLBEntry();
        
        if (index == -1) {
            // TLB full, use LRU replacement
            index = findLRUEntry();
        }
        
        tlb[index].pageNumber = pageNumber;
        tlb[index].frameNumber = frameNumber;
        tlb[index].lastAccessTime = currentTime;
        tlb[index].valid = true;
    }
    
public:
    // Constructor
    TLBSimulator(int size) : tlbSize(size), currentTime(0), 
                             tlbHits(0), tlbMisses(0), totalAccesses(0) {
        tlb.resize(size);
    }
    
    // Initialize page table
    void initializePageTable(const map<int, int>& pt) {
        pageTable = pt;
    }
    
    // Translate virtual address
    int translate(int pageNumber) {
        totalAccesses++;
        currentTime++;
        
        cout << "\nAccess #" << totalAccesses << " - Page: " << pageNumber << " | ";
        
        // Check TLB first
        int tlbIndex = findInTLB(pageNumber);
        
        if (tlbIndex != -1) {
            // TLB Hit
            tlbHits++;
            tlb[tlbIndex].lastAccessTime = currentTime;
            cout << "TLB HIT (Entry " << tlbIndex << ") | ";
            cout << "Frame: " << tlb[tlbIndex].frameNumber << " | ";
            cout << "Access Time: " << TLB_ACCESS_TIME << " ns";
            return tlb[tlbIndex].frameNumber;
        } else {
            // TLB Miss
            tlbMisses++;
            cout << "TLB MISS | ";
            
            // Check page table
            if (pageTable.find(pageNumber) != pageTable.end()) {
                int frameNumber = pageTable[pageNumber];
                cout << "Page Table Lookup | Frame: " << frameNumber << " | ";
                
                // Add to TLB
                addToTLB(pageNumber, frameNumber);
                
                // Access time: TLB lookup + page table lookup + memory access
                int accessTime = TLB_ACCESS_TIME + MEMORY_ACCESS_TIME;
                cout << "Access Time: " << accessTime << " ns";
                
                return frameNumber;
            } else {
                cout << "PAGE FAULT!";
                return -1;
            }
        }
    }
    
    // Display TLB contents
    void displayTLB() {
        cout << "\n\n=== TLB Contents ===" << endl;
        cout << left << setw(8) << "Entry" 
             << setw(10) << "Page" 
             << setw(10) << "Frame" 
             << setw(15) << "Last Access" 
             << setw(8) << "Valid" << endl;
        cout << string(55, '-') << endl;
        
        for (int i = 0; i < tlbSize; i++) {
            cout << left << setw(8) << i;
            if (tlb[i].valid) {
                cout << setw(10) << tlb[i].pageNumber
                     << setw(10) << tlb[i].frameNumber
                     << setw(15) << tlb[i].lastAccessTime
                     << setw(8) << "Yes";
            } else {
                cout << setw(10) << "-"
                     << setw(10) << "-"
                     << setw(15) << "-"
                     << setw(8) << "No";
            }
            cout << endl;
        }
    }
    
    // Display statistics
    void displayStatistics() {
        cout << "\n=== TLB Statistics ===" << endl;
        cout << "Total Accesses: " << totalAccesses << endl;
        cout << "TLB Hits: " << tlbHits << endl;
        cout << "TLB Misses: " << tlbMisses << endl;
        
        double hitRatio = (totalAccesses > 0) ? (tlbHits * 100.0 / totalAccesses) : 0;
        double missRatio = (totalAccesses > 0) ? (tlbMisses * 100.0 / totalAccesses) : 0;
        
        cout << fixed << setprecision(2);
        cout << "TLB Hit Ratio: " << hitRatio << "%" << endl;
        cout << "TLB Miss Ratio: " << missRatio << "%" << endl;
        
        // Calculate effective access time
        double effectiveAccessTime = (hitRatio / 100.0) * TLB_ACCESS_TIME + 
                                     (missRatio / 100.0) * (TLB_ACCESS_TIME + MEMORY_ACCESS_TIME);
        
        cout << "\nTiming Analysis:" << endl;
        cout << "TLB Access Time: " << TLB_ACCESS_TIME << " ns" << endl;
        cout << "Memory Access Time: " << MEMORY_ACCESS_TIME << " ns" << endl;
        cout << "Effective Access Time: " << fixed << setprecision(2) 
             << effectiveAccessTime << " ns" << endl;
        
        double speedup = (TLB_ACCESS_TIME + MEMORY_ACCESS_TIME) / effectiveAccessTime;
        cout << "Speedup Factor: " << fixed << setprecision(2) << speedup << "x" << endl;
    }
    
    // Process reference string
    void processReferenceString(const vector<int>& refString) {
        cout << "\n=== Processing Reference String ===" << endl;
        cout << "TLB Size: " << tlbSize << " entries" << endl;
        cout << "Reference String: ";
        for (int page : refString) {
            cout << page << " ";
        }
        cout << endl;
        cout << string(70, '-') << endl;
        
        for (int page : refString) {
            translate(page);
        }
    }
};

int main() {
    cout << "=== TLB Simulator ===" << endl;
    
    // Initialize page table (page -> frame mappings)
    map<int, int> pageTable;
    pageTable[0] = 5;
    pageTable[1] = 2;
    pageTable[2] = 10;
    pageTable[3] = 7;
    pageTable[4] = 1;
    pageTable[5] = 3;
    pageTable[6] = 8;
    pageTable[7] = 4;
    pageTable[8] = 9;
    pageTable[9] = 6;
    
    // Test case 1: Small TLB (4 entries)
    cout << "\n*** Test 1: TLB with 4 entries ***" << endl;
    TLBSimulator tlb1(4);
    tlb1.initializePageTable(pageTable);
    
    vector<int> refString1 = {0, 1, 2, 0, 1, 3, 0, 3, 1, 2, 4, 5, 2, 1, 0};
    tlb1.processReferenceString(refString1);
    tlb1.displayTLB();
    tlb1.displayStatistics();
    
    // Test case 2: Larger TLB (8 entries)
    cout << "\n\n*** Test 2: TLB with 8 entries ***" << endl;
    TLBSimulator tlb2(8);
    tlb2.initializePageTable(pageTable);
    tlb2.processReferenceString(refString1);
    tlb2.displayTLB();
    tlb2.displayStatistics();
    
    // Test case 3: Comparison with different TLB sizes
    cout << "\n\n=== TLB Size Comparison ===" << endl;
    vector<int> refString2 = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5, 1, 2, 3};
    
    cout << "\n" << left << setw(12) << "TLB Size" 
         << setw(12) << "Hits" 
         << setw(12) << "Misses" 
         << setw(15) << "Hit Ratio" 
         << setw(20) << "Effective Time" << endl;
    cout << string(75, '-') << endl;
    
    for (int size = 2; size <= 8; size += 2) {
        TLBSimulator tlb(size);
        tlb.initializePageTable(pageTable);
        
        for (int page : refString2) {
            tlb.translate(page);
        }
        
        // Calculate statistics
        int hits = 0, misses = 0;
        for (int page : refString2) {
            // Simplified - just for comparison
        }
        
        // Get stats from simulator
        cout << left << setw(12) << size << " entries" << endl;
    }
    
    // Demonstrate locality of reference
    cout << "\n\n*** Test 3: Demonstrating Locality of Reference ***" << endl;
    TLBSimulator tlb3(6);
    tlb3.initializePageTable(pageTable);
    
    // High locality reference string
    vector<int> highLocality = {0, 0, 1, 0, 1, 2, 0, 1, 2, 0, 1, 2, 3, 0, 1};
    cout << "\nHigh Locality Pattern:" << endl;
    tlb3.processReferenceString(highLocality);
    tlb3.displayStatistics();
    
    // Low locality reference string
    cout << "\n\nLow Locality Pattern:" << endl;
    TLBSimulator tlb4(6);
    tlb4.initializePageTable(pageTable);
    vector<int> lowLocality = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4};
    tlb4.processReferenceString(lowLocality);
    tlb4.displayStatistics();
    
    return 0;
}
