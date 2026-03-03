#include <iostream>
#include <bitset>
#include <thread>
#include <chrono>
#include <queue>
#include <functional>
#include <mutex>
#include <string>

using namespace std;

// Status Register
struct StatusRegister {
    bool busy = false;
    bool error = false;
    bool ready = true;
    bool transferComplete = false;
};

// IOPort class
class IOPort {
public:
    uint8_t dataRegister;
    StatusRegister status;
    uint8_t controlRegister;

    IOPort() {
        dataRegister = 0;
        controlRegister = 0;
    }

    void writeData(uint8_t data) {
        if (status.busy) {
            cout << "Error: Device Busy!" << endl;
            status.error = true;
            return;
        }

        status.busy = true;
        status.ready = false;

        cout << "Writing data: " << (int)data << endl;

        this_thread::sleep_for(chrono::seconds(1));

        dataRegister = data;

        status.busy = false;
        status.ready = true;
        status.transferComplete = true;
    }

    uint8_t readData() {
        return dataRegister;   // ✅ FIXED
    }

    bool pollStatus() {
        return status.ready;   // ✅ FIXED
    }
};

// DeviceController class
class DeviceController {
private:
    IOPort port;
    string deviceName;
    queue<function<void()>> interruptQueue;

public:
    DeviceController(string name) {
        deviceName = name;
    }

    void pollingIO(uint8_t data) {
        cout << "[POLLING] Waiting for device ready..." << endl;

        while (!port.pollStatus()) {
            cout << "Device busy..." << endl;
        }

        port.writeData(data);
        cout << "Polling transfer complete!\n";
    }

    void interruptDrivenIO(uint8_t data, function<void()> callback) {
        cout << "[INTERRUPT] Sending data..." << endl;

        port.writeData(data);

        interruptQueue.push(callback);
    }

    void processInterrupts() {
        while (!interruptQueue.empty()) {
            interruptQueue.front()();
            interruptQueue.pop();
        }
    }
};

int main() {
    cout << "=== I/O Hardware Simulation ===" << endl;

    DeviceController keyboard("Keyboard");
    DeviceController disk("Disk Drive");

    cout << "\n--- Polling I/O Test ---" << endl;
    keyboard.pollingIO(10);

    cout << "\n--- Interrupt-driven I/O Test ---" << endl;
    disk.interruptDrivenIO(20, []() {
        cout << "Interrupt: Transfer completed!" << endl;
    });

    disk.processInterrupts();

    return 0;
}