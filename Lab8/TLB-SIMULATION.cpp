#include <iostream>
#include <list>
#include <unordered_map>
#include <iomanip>

using namespace std;

const int TLB_SIZE = 8;

/* =========================
   TLB (LRU Replacement)
   ========================= */
class TLB {
private:
    struct Entry {
        int page;
        int frame;
        Entry(int p, int f) : page(p), frame(f) {}
    };

    list<Entry> cache;   // MRU at front
    unordered_map<int, list<Entry>::iterator> index;

    int hits  = 0;
    int misses = 0;

public:
    int lookup(int page) {
        auto it = index.find(page);

        if (it == index.end()) {
            misses++;
            cout << "TLB MISS: Page " << page << endl;
            return -1;
        }

        // TLB hit
        hits++;
        int frame = it->second->frame;

        // Move to MRU position
        cache.splice(cache.begin(), cache, it->second);

        cout << "TLB HIT : Page " << page
             << " -> Frame " << frame << endl;

        return frame;
    }

    void insert(int page, int frame) {
        auto it = index.find(page);

        // Update existing entry
        if (it != index.end()) {
            it->second->frame = frame;
            cache.splice(cache.begin(), cache, it->second);
            return;
        }

        // Evict LRU if full
        if (cache.size() == TLB_SIZE) {
            int lruPage = cache.back().page;
            cout << "TLB FULL: Evicting Page " << lruPage << endl;
            index.erase(lruPage);
            cache.pop_back();
        }

        // Insert new MRU entry
        cache.emplace_front(page, frame);
        index[page] = cache.begin();

        cout << "TLB INSERT: Page " << page
             << " -> Frame " << frame << endl;
    }

    void display() const {
        cout << "\n=== TLB CONTENTS (MRU → LRU) ===\n";
        cout << setw(10) << "Page"
             << setw(10) << "Frame\n";
        cout << string(20, '-') << endl;

        for (const auto& e : cache) {
            cout << setw(10) << e.page
                 << setw(10) << e.frame << endl;
        }
    }

    void stats() const {
        int total = hits + misses;
        double hitRate = total ? (100.0 * hits / total) : 0.0;

        cout << "\n=== TLB STATISTICS ===\n";
        cout << "Accesses : " << total << endl;
        cout << "Hits     : " << hits << endl;
        cout << "Misses   : " << misses << endl;
        cout << "Hit Rate : " << fixed << setprecision(2)
             << hitRate << "%\n";
    }
};

/* =========================
   Memory System
   ========================= */
class MemorySystem {
private:
    TLB tlb;
    unordered_map<int, int> pageTable;

public:
    MemorySystem() {
        // Simple static page table
        pageTable = {
            {0,5}, {1,2}, {2,8}, {3,1},
            {4,9}, {5,3}, {6,7}, {7,4}
        };
    }

    int translate(int page) {
        cout << "\n--- Access Page " << page << " ---\n";

        // 1. TLB lookup
        int frame = tlb.lookup(page);
        if (frame != -1)
            return frame;

        // 2. Page table lookup
        auto it = pageTable.find(page);
        if (it == pageTable.end()) {
            cout << "PAGE FAULT: Page " << page << endl;
            return -1;
        }

        frame = it->second;
        cout << "Page Table: Page " << page
             << " -> Frame " << frame << endl;

        // 3. Update TLB
        tlb.insert(page, frame);
        return frame;
    }

    void displayStatus() const {
        tlb.display();
        tlb.stats();
    }
};

/* =========================
   Main
   ========================= */
int main() {
    cout << "TLB SIMULATION (LRU)\n";
    cout << "===================\n";
    cout << "TLB Size: " << TLB_SIZE << " entries\n\n";

    MemorySystem mem;

    int references[] = {
        0,1,2,3,0,1,4,0,
        1,2,3,5,6,7,0,1
    };

    int n = sizeof(references) / sizeof(references[0]);

    cout << "Reference String: ";
    for (int i = 0; i < n; i++)
        cout << references[i] << " ";
    cout << "\n";

    for (int i = 0; i < n; i++)
        mem.translate(references[i]);

    mem.displayStatus();
    return 0;
}
