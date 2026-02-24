#include <iostream>
#include <vector>
#include <bitset>
using namespace std;

const int TOTAL_BLOCKS = 16;

class DiskManager {
    bitset<TOTAL_BLOCKS> freeBlocks;

public:
    DiskManager() {
        freeBlocks.set();  // all blocks are free
    }

    int allocate() {
        for (int i = 0; i < TOTAL_BLOCKS; i++) {
            if (freeBlocks[i]) {
                freeBlocks[i] = 0;
                return i;
            }
        }
        return -1;  // disk full
    }

    void release(int block) {
        freeBlocks[block] = 1;
    }

    void status() {
        cout << "Free blocks: "
             << freeBlocks.count()
             << "/" << TOTAL_BLOCKS
             << endl;
    }
};

int main() {
    DiskManager dm;

    int b1 = dm.allocate();
    int b2 = dm.allocate();

    dm.status();

    dm.release(b1);

    dm.status();

    return 0;
}