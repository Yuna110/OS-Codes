#include <iostream>
#include <string>
using namespace std;

class StorageDevice {
public:
    virtual string getInterface() = 0;
    virtual double getBandwidth() = 0; // MB/s

    virtual void read(int lba) {
        cout << getInterface()
             << ": Reading LBA "
             << lba
             << " at "
             << getBandwidth()
             << " MB/s"
             << endl;
    }

    virtual ~StorageDevice() {}
};

class SATADrive : public StorageDevice {
public:
    string getInterface() override { return "SATA"; }
    double getBandwidth() override { return 600.0; }
};

class NVMeDrive : public StorageDevice {
public:
    string getInterface() override { return "NVMe"; }
    double getBandwidth() override { return 7000.0; }
};

int main() {
    StorageDevice* d1 = new SATADrive();
    StorageDevice* d2 = new NVMeDrive();

    d1->read(100);
    d2->read(100);

    delete d1;
    delete d2;

    return 0;
}