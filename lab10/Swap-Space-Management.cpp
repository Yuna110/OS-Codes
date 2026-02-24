#include <iostream>
#include <map>
using namespace std;

class SwapManager {
    int swapSize;   // in MB
    int usedSwap;
    map<int, int> swapTable;   // pid -> MB used

public:
    SwapManager(int size) : swapSize(size), usedSwap(0) {}

    bool swapOut(int pid, int mb) {
        if (usedSwap + mb > swapSize)
            return false;

        swapTable[pid] = mb;
        usedSwap += mb;

        cout << "Process " << pid
             << " swapped out ("
             << mb << "MB)" << endl;

        return true;
    }

    void swapIn(int pid) {
        if (swapTable.count(pid)) {
            usedSwap -= swapTable[pid];
            swapTable.erase(pid);

            cout << "Process " << pid
                 << " swapped back in" << endl;
        }
    }
};

int main() {
    SwapManager sm(512);

    sm.swapOut(1, 128);
    sm.swapOut(2, 256);
    sm.swapOut(3, 200);  // should fail
    sm.swapIn(1);
    sm.swapOut(3, 200);  // now succeeds

    return 0;
}