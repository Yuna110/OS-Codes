#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <stdexcept>
#include <queue>

using namespace std;

// ============================================
// Base Class: IODevice
// ============================================
class IODevice {
protected:
    string deviceName;
    bool isOpen;

public:
    IODevice(const string& name) : deviceName(name), isOpen(false) {}
    virtual ~IODevice() = default;

    virtual bool open() = 0;
    virtual void close() = 0;
    virtual string getDeviceType() const = 0;
    virtual void displayInfo() const = 0;

    string getName() const { return deviceName; }
    bool getStatus() const { return isOpen; }
};

// ============================================
// Block Device
// ============================================
class BlockDevice : public IODevice {
private:
    size_t blockSize;
    size_t totalBlocks;
    vector<vector<uint8_t>> blocks;

public:
    BlockDevice(const string& name, size_t blockSz, size_t totalBlk)
        : IODevice(name), blockSize(blockSz), totalBlocks(totalBlk),
          blocks(totalBlk, vector<uint8_t>(blockSz, 0)) {}

    bool open() override {
        isOpen = true;
        cout << "[Block Device] " << deviceName << " opened\n";
        return true;
    }

    void close() override {
        isOpen = false;
        cout << "[Block Device] " << deviceName << " closed\n";
    }

    vector<uint8_t> readBlock(size_t blockNum) {
        if (!isOpen) throw runtime_error("Device not open");
        if (blockNum >= totalBlocks) throw out_of_range("Block out of range");
        return blocks[blockNum];
    }

    void writeBlock(size_t blockNum, const vector<uint8_t>& data) {
        if (!isOpen) throw runtime_error("Device not open");
        if (blockNum >= totalBlocks) throw out_of_range("Block out of range");
        if (data.size() != blockSize)
            throw invalid_argument("Data size mismatch");

        blocks[blockNum] = data;
        cout << "[Block Device] Block " << blockNum << " written\n";
    }

    string getDeviceType() const override { return "Block Device"; }

    void displayInfo() const override {
        cout << "Device: " << deviceName << "\n"
             << "Type: " << getDeviceType() << "\n"
             << "Block Size: " << blockSize << " bytes\n"
             << "Total Blocks: " << totalBlocks << "\n"
             << "Total Capacity: " << blockSize * totalBlocks << " bytes\n";
    }
};

// ============================================
// Character Device
// ============================================
class CharacterDevice : public IODevice {
private:
    queue<char> inputBuffer;
    string outputBuffer;

public:
    CharacterDevice(const string& name) : IODevice(name) {}

    bool open() override {
        isOpen = true;
        cout << "[Char Device] " << deviceName << " opened\n";
        return true;
    }

    void close() override {
        isOpen = false;
        cout << "[Char Device] " << deviceName << " closed\n";
    }

    char getChar() {
        if (!isOpen) throw runtime_error("Device not open");
        if (inputBuffer.empty()) return '\0';

        char c = inputBuffer.front();
        inputBuffer.pop();
        return c;
    }

    void putChar(char c) {
        if (!isOpen) throw runtime_error("Device not open");
        outputBuffer += c;
        cout << "[Char Device] Output: " << c << "\n";
    }

    void simulateInput(const string& input) {
        for (char c : input)
            inputBuffer.push(c);
    }

    string getDeviceType() const override { return "Character Device"; }

    void displayInfo() const override {
        cout << "Device: " << deviceName << "\n"
             << "Type: " << getDeviceType() << "\n"
             << "Buffer size: " << inputBuffer.size() << " chars\n";
    }
};

// ============================================
// Network Device
// ============================================
class NetworkDevice : public IODevice {
private:
    string ipAddress;
    int port;
    bool connected;
    vector<string> receivedPackets;

public:
    NetworkDevice(const string& name, const string& ip, int p)
        : IODevice(name), ipAddress(ip), port(p), connected(false) {}

    bool open() override {
        isOpen = true;
        cout << "[Network Device] " << deviceName << " opened\n";
        return true;
    }

    void close() override {
        isOpen = false;
        connected = false;
        cout << "[Network Device] " << deviceName << " closed\n";
    }

    bool connect() {
        if (!isOpen) return false;
        connected = true;
        cout << "[Network] Connected to "
             << ipAddress << ":" << port << "\n";
        return true;
    }

    void sendPacket(const string& data) {
        if (!connected)
            throw runtime_error("Not connected");
        cout << "[Network] Sending: " << data << "\n";
    }

    void receivePacket(const string& packet) {
        receivedPackets.push_back(packet);
    }

    string getDeviceType() const override { return "Network Device"; }

    void displayInfo() const override {
        cout << "Device: " << deviceName << "\n"
             << "Type: " << getDeviceType() << "\n"
             << "IP: " << ipAddress << ":" << port << "\n"
             << "Connected: " << (connected ? "Yes" : "No") << "\n";
    }
};

// ============================================
// Device Manager
// ============================================
class DeviceManager {
private:
    map<string, shared_ptr<IODevice>> deviceTable;

public:
    void registerDevice(shared_ptr<IODevice> device) {
        deviceTable[device->getName()] = device;
        cout << "[DevMgr] Registered: "
             << device->getName()
             << " (" << device->getDeviceType() << ")\n";
    }

    shared_ptr<IODevice> getDevice(const string& name) {
        if (deviceTable.find(name) == deviceTable.end())
            throw runtime_error("Device not found: " + name);
        return deviceTable.at(name);
    }

    void listAllDevices() const {
        cout << "\n=== Registered Devices ===\n";
        for (const auto& pair : deviceTable) {
            cout << "- " << pair.first
                 << " [" << pair.second->getDeviceType() << "]"
                 << " Status: "
                 << (pair.second->getStatus() ? "Open" : "Closed")
                 << "\n";
        }
    }

    void openDevice(const string& name) {
        getDevice(name)->open();
    }

    void closeDevice(const string& name) {
        getDevice(name)->close();
    }
};

// ============================================
// Main
// ============================================
int main() {
    cout << "=== Application I/O Interface Demo ===\n";

    DeviceManager devMgr;

    auto disk = make_shared<BlockDevice>("sda", 512, 100);
    auto keyboard = make_shared<CharacterDevice>("keyboard");
    auto ethernet = make_shared<NetworkDevice>("eth0", "192.168.1.1", 8080);

    devMgr.registerDevice(disk);
    devMgr.registerDevice(keyboard);
    devMgr.registerDevice(ethernet);

    devMgr.listAllDevices();

    cout << "\n--- Block Device Test ---\n";
    devMgr.openDevice("sda");
    auto diskDevice =
        dynamic_pointer_cast<BlockDevice>(devMgr.getDevice("sda"));

    vector<uint8_t> testData(512, 0xAB);
    diskDevice->writeBlock(0, testData);

    auto readData = diskDevice->readBlock(0);
    cout << "Read block 0, first byte: "
         << hex << (int)readData[0] << dec << "\n";

    diskDevice->displayInfo();

    cout << "\n--- Character Device Test ---\n";
    devMgr.openDevice("keyboard");
    auto kbDevice =
        dynamic_pointer_cast<CharacterDevice>(devMgr.getDevice("keyboard"));

    kbDevice->simulateInput("Hello OS!");

    for (int i = 0; i < 5; i++) {
        char c = kbDevice->getChar();
        if (c != '\0')
            cout << "Read char: " << c << "\n";
    }

    cout << "\n--- Network Device Test ---\n";
    devMgr.openDevice("eth0");
    auto netDevice =
        dynamic_pointer_cast<NetworkDevice>(devMgr.getDevice("eth0"));

    netDevice->connect();
    netDevice->sendPacket("GET / HTTP/1.1");

    devMgr.listAllDevices();

    return 0;
}