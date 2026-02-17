/*
 * Exercise 8: Optimal Page Replacement Algorithm
 * Implements Belady's optimal algorithm and compares with FIFO and LRU
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <limits>
#include <map>

using namespace std;

class OptimalPageReplacement {
private:
    int numFrames;
    vector<int> frames;
    int pageFaults;
    const vector<int>* refString;  // Pointer to reference string
    
    bool isPageInMemory(int page) {
        return find(frames.begin(), frames.end(), page) != frames.end();
    }
    
    int findEmptyFrame() {
        for (int i = 0; i < numFrames; i++) {
            if (frames[i] == -1) return i;
        }
        return -1;
    }
    
    // Find page that will not be used for longest time
    int findOptimalVictim(int currentIndex) {
        int victimIndex = -1;
        int farthest = currentIndex;
        
        for (int i = 0; i < numFrames; i++) {
            int page = frames[i];
            
            // Find next use of this page
            int nextUse = numeric_limits<int>::max();
            for (int j = currentIndex + 1; j < refString->size(); j++) {
                if ((*refString)[j] == page) {
                    nextUse = j;
                    break;
                }
            }
            
            // Replace page that won't be used for longest time
            if (nextUse > farthest) {
                farthest = nextUse;
                victimIndex = i;
            }
        }
        
        return victimIndex;
    }
    
public:
    OptimalPageReplacement(int frames) : numFrames(frames), pageFaults(0) {
        this->frames.resize(frames, -1);
    }
    
    void processReferenceString(const vector<int>& refStr) {
        refString = &refStr;
        frames.assign(numFrames, -1);
        pageFaults = 0;
        
        for (int i = 0; i < refStr.size(); i++) {
            int page = refStr[i];
            
            if (!isPageInMemory(page)) {
                pageFaults++;
                
                int emptyFrame = findEmptyFrame();
                if (emptyFrame != -1) {
                    frames[emptyFrame] = page;
                } else {
                    int victimIndex = findOptimalVictim(i);
                    frames[victimIndex] = page;
                }
            }
        }
    }
    
    int getPageFaults() const { return pageFaults; }
};

class FIFOPageReplacement {
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
    FIFOPageReplacement(int frames) : numFrames(frames), pageFaults(0) {
        this->frames.resize(frames, -1);
    }
    
    void processReferenceString(const vector<int>& refString) {
        frames.assign(numFrames, -1);
        insertOrder.clear();
        pageFaults = 0;
        
        for (int page : refString) {
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
    }
    
    int getPageFaults() const { return pageFaults; }
};

class LRUPageReplacement {
private:
    int numFrames;
    vector<int> frames;
    map<int, int> lastAccessTime;
    int currentTime;
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
    LRUPageReplacement(int frames) : numFrames(frames), currentTime(0), pageFaults(0) {
        this->frames.resize(frames, -1);
    }
    
    void processReferenceString(const vector<int>& refString) {
        frames.assign(numFrames, -1);
        lastAccessTime.clear();
        currentTime = 0;
        pageFaults = 0;
        
        for (int page : refString) {
            currentTime++;
            lastAccessTime[page] = currentTime;
            
            if (!isPageInMemory(page)) {
                pageFaults++;
                
                int emptyFrame = findEmptyFrame();
                if (emptyFrame != -1) {
                    frames[emptyFrame] = page;
                } else {
                    int lruPage = findLRUPage();
                    
                    for (int i = 0; i < numFrames; i++) {
                        if (frames[i] == lruPage) {
                            frames[i] = page;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    int getPageFaults() const { return pageFaults; }
};

void compareAlgorithms(const vector<int>& refString, int numFrames) {
    cout << "\n=== Comparison Report ===" << endl;
    cout << "Reference String: ";
    for (int page : refString) {
        cout << page << " ";
    }
    cout << "\nNumber of Frames: " << numFrames << endl;
    cout << string(70, '-') << endl;
    
    OptimalPageReplacement optimal(numFrames);
    optimal.processReferenceString(refString);
    
    FIFOPageReplacement fifo(numFrames);
    fifo.processReferenceString(refString);
    
    LRUPageReplacement lru(numFrames);
    lru.processReferenceString(refString);
    
    cout << "\n" << left << setw(20) << "Algorithm" 
         << setw(15) << "Page Faults" 
         << setw(20) << "vs Optimal" 
         << setw(15) << "Performance" << endl;
    cout << string(70, '-') << endl;
    
    cout << left << setw(20) << "Optimal"
         << setw(15) << optimal.getPageFaults()
         << setw(20) << "-"
         << setw(15) << "100%" << endl;
    
    double fifoPerf = (optimal.getPageFaults() * 100.0) / fifo.getPageFaults();
    cout << left << setw(20) << "FIFO"
         << setw(15) << fifo.getPageFaults()
         << setw(20) << ("+" + to_string(fifo.getPageFaults() - optimal.getPageFaults()) + " faults")
         << setw(15) << (to_string((int)fifoPerf) + "%") << endl;
    
    double lruPerf = (optimal.getPageFaults() * 100.0) / lru.getPageFaults();
    cout << left << setw(20) << "LRU"
         << setw(15) << lru.getPageFaults()
         << setw(20) << ("+" + to_string(lru.getPageFaults() - optimal.getPageFaults()) + " faults")
         << setw(15) << (to_string((int)lruPerf) + "%") << endl;
    
    cout << string(70, '-') << endl;
    
    // Determine best practical algorithm
    if (lru.getPageFaults() < fifo.getPageFaults()) {
        cout << "Best practical algorithm: LRU (closest to optimal)" << endl;
    } else if (fifo.getPageFaults() < lru.getPageFaults()) {
        cout << "Best practical algorithm: FIFO (closest to optimal)" << endl;
    } else {
        cout << "Both FIFO and LRU perform equally" << endl;
    }
}

int main() {
    cout << "=== Optimal Page Replacement Algorithm ===" << endl;
    
    // Test case 1: Standard comparison
    cout << "\n*** Test 1: Standard Reference String ***" << endl;
    vector<int> refString1 = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2};
    compareAlgorithms(refString1, 3);
    
    // Test case 2: Another example
    cout << "\n\n*** Test 2: Another Reference String ***" << endl;
    vector<int> refString2 = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    compareAlgorithms(refString2, 4);
    
    // Test case 3: High locality
    cout << "\n\n*** Test 3: High Locality Pattern ***" << endl;
    vector<int> refString3 = {1, 2, 1, 2, 1, 3, 1, 2, 1, 2, 3, 1, 2};
    compareAlgorithms(refString3, 3);
    
    // Test case 4: Worst case for FIFO (Belady's Anomaly)
    cout << "\n\n*** Test 4: Belady's Anomaly Demonstration ***" << endl;
    vector<int> refString4 = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    
    cout << "\nWith 3 frames:" << endl;
    compareAlgorithms(refString4, 3);
    
    cout << "\nWith 4 frames:" << endl;
    compareAlgorithms(refString4, 4);
    
    // Test case 5: Multiple frame sizes comparison
    cout << "\n\n*** Test 5: Effect of Frame Count ***" << endl;
    vector<int> refString5 = {0, 1, 2, 3, 0, 1, 4, 0, 1, 2, 3, 4};
    
    cout << "\n" << left << setw(10) << "Frames"
         << setw(12) << "Optimal"
         << setw(12) << "FIFO"
         << setw(12) << "LRU" << endl;
    cout << string(50, '-') << endl;
    
    for (int frames = 2; frames <= 6; frames++) {
        OptimalPageReplacement opt(frames);
        opt.processReferenceString(refString5);
        
        FIFOPageReplacement fifo(frames);
        fifo.processReferenceString(refString5);
        
        LRUPageReplacement lru(frames);
        lru.processReferenceString(refString5);
        
        cout << left << setw(10) << frames
             << setw(12) << opt.getPageFaults()
             << setw(12) << fifo.getPageFaults()
             << setw(12) << lru.getPageFaults() << endl;
    }
    
    // Test case 6: Why Optimal is theoretical
    cout << "\n\n*** Test 6: Why Optimal is Theoretical ***" << endl;
    cout << "\nThe Optimal algorithm requires:" << endl;
    cout << "1. Complete knowledge of future page references" << endl;
    cout << "2. Ability to predict which page won't be used for longest time" << endl;
    cout << "3. This information is not available in real systems" << endl;
    cout << "\nOptimal is used as a benchmark to evaluate practical algorithms." << endl;
    cout << "LRU approximates Optimal by assuming past behavior predicts future." << endl;
    
    return 0;
}
