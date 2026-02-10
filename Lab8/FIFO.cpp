#include <iostream>
#include <vector>
#include <iomanip>
using namespace std;

const int TOTAL_MEMORY = 1048576; // 1 MB

struct MemoryBlock {
    int start;
    int size;
    bool free;
    int pid;

    MemoryBlock(int s, int sz, bool f = true, int p = -1)
        : start(s), size(sz), free(f), pid(p) {}
};

enum class FitStrategy {
    FIRST,
    BEST,
    WORST
};

class MemoryManager {
private:
    vector<MemoryBlock> blocks;

    void mergeFreeBlocks() {
        for (size_t i = 0; i + 1 < blocks.size();) {
            if (blocks[i].free && blocks[i + 1].free) {
                blocks[i].size += blocks[i + 1].size;
                blocks.erase(blocks.begin() + i + 1);
            } else {
                i++;
            }
        }
    }

    int findBlock(int size, FitStrategy strategy) {
        int index = -1;

        if (strategy == FitStrategy::FIRST) {
            for (size_t i = 0; i < blocks.size(); i++) {
                if (blocks[i].free && blocks[i].size >= size)
                    return i;
            }
        }
        else if (strategy == FitStrategy::BEST) {
            int bestSize = TOTAL_MEMORY + 1;
            for (size_t i = 0; i < blocks.size(); i++) {
                if (blocks[i].free && blocks[i].size >= size &&
                    blocks[i].size < bestSize) {
                    bestSize = blocks[i].size;
                    index = i;
                }
            }
        }
        else { // WORST
            int worstSize = -1;
            for (size_t i = 0; i < blocks.size(); i++) {
                if (blocks[i].free && blocks[i].size >= size &&
                    blocks[i].size > worstSize) {
                    worstSize = blocks[i].size;
                    index = i;
                }
            }
        }
        return index;
    }

public:
    MemoryManager() {
        blocks.emplace_back(0, TOTAL_MEMORY);
    }

    bool allocate(int pid, int size, FitStrategy strategy) {
        static const char* names[] = {"First-Fit", "Best-Fit", "Worst-Fit"};

        cout << "\n--- " << names[(int)strategy] << " Allocation ---\n";
        cout << "Process " << pid << " requests " << size << " bytes\n";

        int idx = findBlock(size, strategy);
        if (idx == -1) {
            cout << "Allocation FAILED\n";
            return false;
        }

        MemoryBlock& block = blocks[idx];
        cout << "Using block at address " << block.start
             << " (size " << block.size << ")\n";

        if (block.size > size) {
            blocks.insert(blocks.begin() + idx + 1,
                MemoryBlock(block.start + size, block.size - size));
        }

        block.size = size;
        block.free = false;
        block.pid = pid;

        cout << "Allocated at address " << block.start << endl;
        return true;
    }

    void deallocate(int pid) {
        cout << "\n--- Deallocation ---\n";
        bool found = false;

        for (auto& b : blocks) {
            if (!b.free && b.pid == pid) {
                b.free = true;
                b.pid = -1;
                found = true;
                cout << "Freed block at address " << b.start << endl;
            }
        }

        if (!found) {
            cout << "Process not found\n";
            return;
        }

        mergeFreeBlocks();
    }

    void display() const {
        cout << "\n=== MEMORY MAP ===\n";
        cout << setw(12) << "Start"
             << setw(10) << "Size"
             << setw(10) << "Status"
             << setw(10) << "PID\n";
        cout << string(42, '-') << endl;

        for (const auto& b : blocks) {
            cout << setw(12) << b.start
                 << setw(10) << b.size
                 << setw(10) << (b.free ? "FREE" : "USED")
                 << setw(10) << (b.free ? "-" : to_string(b.pid)) << endl;
        }
    }

    void fragmentation() const {
        int totalFree = 0, largestFree = 0, freeBlocks = 0, used = 0;

        for (const auto& b : blocks) {
            if (b.free) {
                totalFree += b.size;
                freeBlocks++;
                largestFree = max(largestFree, b.size);
            } else {
                used += b.size;
            }
        }

        int externalFrag = totalFree - largestFree;
        double percent = totalFree ?
            (double)externalFrag / totalFree * 100 : 0;

        cout << "\n=== FRAGMENTATION ===\n";
        cout << "Total Free Space   : " << totalFree << endl;
        cout << "Largest Free Block : " << largestFree << endl;
        cout << "Free Blocks        : " << freeBlocks << endl;
        cout << "Allocated Space   : " << used << endl;
        cout << "External Fragment : " << externalFrag << endl;
        cout << "Fragmentation %   : "
             << fixed << setprecision(2) << percent << "%\n";
    }
};

/* =====================
   Main Test Driver
   ===================== */
int main() {
    cout << "MEMORY ALLOCATION SIMULATOR\n";
    cout << "===========================\n";

    // First Fit
    cout << "\n========== FIRST-FIT ==========";
    MemoryManager mm1;
    mm1.allocate(1, 200000, FitStrategy::FIRST);
    mm1.allocate(2, 150000, FitStrategy::FIRST);
    mm1.allocate(3, 300000, FitStrategy::FIRST);
    mm1.display();
    mm1.deallocate(2);
    mm1.allocate(4, 100000, FitStrategy::FIRST);
    mm1.display();
    mm1.fragmentation();

    // Best Fit
    cout << "\n========== BEST-FIT ==========";
    MemoryManager mm2;
    mm2.allocate(1, 200000, FitStrategy::BEST);
    mm2.allocate(2, 150000, FitStrategy::BEST);
    mm2.allocate(3, 300000, FitStrategy::BEST);
    mm2.deallocate(2);
    mm2.allocate(4, 100000, FitStrategy::BEST);
    mm2.display();
    mm2.fragmentation();

    // Worst Fit
    cout << "\n========== WORST-FIT ==========";
    MemoryManager mm3;
    mm3.allocate(1, 200000, FitStrategy::WORST);
    mm3.allocate(2, 150000, FitStrategy::WORST);
    mm3.allocate(3, 300000, FitStrategy::WORST);
    mm3.deallocate(2);
    mm3.allocate(4, 100000, FitStrategy::WORST);
    mm3.display();
    mm3.fragmentation();

    return 0;
}
