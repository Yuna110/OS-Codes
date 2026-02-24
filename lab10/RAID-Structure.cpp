#include <iostream>
#include <vector>
#include <string>
using namespace std;

class RAID {
    int numDisks;
    vector<vector<string>> disks;

public:
    RAID(int n) : numDisks(n), disks(n) {}

    // RAID-0: stripe data across disks
    void writeRAID0(string data) {
        for (int i = 0; i < data.size(); i++)
            disks[i % numDisks].push_back(string(1, data[i]));
    }

    // RAID-1: mirror data to all disks
    void writeRAID1(string data) {
        for (auto& disk : disks)
            disk.push_back(data);
    }

    void status() {
        for (int i = 0; i < numDisks; i++) {
            cout << "Disk " << i << ": ";
            for (auto& b : disks[i])
                cout << b << " ";
            cout << endl;
        }
    }
};

int main() {
    RAID r0(3);
    r0.writeRAID0("ABCDEF");

    cout << "=== RAID-0 ===" << endl;
    r0.status();

    RAID r1(3);
    r1.writeRAID1("BACKUP");

    cout << "=== RAID-1 ===" << endl;
    r1.status();

    return 0;
}