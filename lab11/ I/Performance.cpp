#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>   // for memcpy
#include <cstdlib>   // for rand, srand

using namespace std;

// ============================================
// Performance Metrics Structure
// ============================================
struct IOMetrics {
    string testName;
    double throughput = 0;
    double latency = 0;
    double iops = 0;
    long long totalBytes = 0;
    int totalOperations = 0;
    double duration = 0;

    void display() const {
        cout << "\n=== " << testName << " ===\n";
        cout << fixed << setprecision(2);
        cout << "Throughput:   " << throughput / 1024 / 1024 << " MB/s\n";
        cout << "Latency:      " << latency << " ms\n";
        cout << "IOPS:         " << iops << " ops/sec\n";
        cout << "Total Data:   " << totalBytes / 1024 << " KB\n";
        cout << "Operations:   " << totalOperations << "\n";
        cout << "Duration:     " << duration << " sec\n";
    }
};

// ============================================
// Simulated Disk
// ============================================
class SimulatedDisk {
private:
    vector<vector<uint8_t>> sectors;
    size_t sectorSize;
    size_t numSectors;
    int currentHead;

public:
    SimulatedDisk(size_t sectorSz, size_t numSect)
        : sectorSize(sectorSz),
          numSectors(numSect),
          currentHead(0) {
        sectors.resize(numSectors, vector<uint8_t>(sectorSize, 0));
    }

    double sequentialWrite(size_t numOps) {
        auto start = chrono::high_resolution_clock::now();

        for (size_t i = 0; i < numOps && i < numSectors; i++) {
            currentHead = i;
            vector<uint8_t> data(sectorSize, i % 256);
            sectors[i] = data;
        }

        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double>(end - start).count();
    }

    double sequentialRead(size_t numOps) {
        auto start = chrono::high_resolution_clock::now();

        for (size_t i = 0; i < numOps && i < numSectors; i++) {
            currentHead = i;
            volatile uint8_t dummy = sectors[i][0];
            (void)dummy;
        }

        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double>(end - start).count();
    }

    double randomWrite(size_t numOps) {
        auto start = chrono::high_resolution_clock::now();

        srand(42);
        for (size_t i = 0; i < numOps; i++) {
            size_t target = rand() % numSectors;
            currentHead = target;
            vector<uint8_t> data(sectorSize, i % 256);
            sectors[target] = data;
        }

        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double>(end - start).count();
    }

    double randomRead(size_t numOps) {
        auto start = chrono::high_resolution_clock::now();

        srand(42);
        for (size_t i = 0; i < numOps; i++) {
            size_t target = rand() % numSectors;
            currentHead = target;
            volatile uint8_t dummy = sectors[target][0];
            (void)dummy;
        }

        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double>(end - start).count();
    }

    size_t getSectorSize() const { return sectorSize; }
};

// ============================================
// Performance Report
// ============================================
class PerformanceReport {
private:
    vector<IOMetrics> results;

public:
    void addResult(const IOMetrics& m) {
        results.push_back(m);
    }

    void generateReport() const {
        cout << "\n===== PERFORMANCE SUMMARY =====\n";
        cout << left << setw(20) << "Test"
             << right << setw(12) << "MB/s"
             << setw(12) << "Latency"
             << setw(12) << "IOPS\n";

        for (const auto& m : results) {
            cout << left << setw(20) << m.testName
                 << right << fixed << setprecision(2)
                 << setw(12) << m.throughput / 1024 / 1024
                 << setw(12) << m.latency
                 << setw(12) << (int)m.iops << "\n";
        }
    }
};

// ============================================
// MAIN
// ============================================
int main() {

    cout << "=== I/O Performance Measurement ===\n";

    PerformanceReport report;
    SimulatedDisk disk(512, 10000);
    const size_t NUM_OPS = 5000;

    // =========================
    // Sequential Write
    // =========================
    double seqWriteTime = disk.sequentialWrite(NUM_OPS);

    IOMetrics seqWrite;
    seqWrite.testName = "Seq Write";
    seqWrite.totalBytes = NUM_OPS * disk.getSectorSize();
    seqWrite.totalOperations = NUM_OPS;
    seqWrite.duration = seqWriteTime;

    if (seqWriteTime > 0) {
        seqWrite.throughput = seqWrite.totalBytes / seqWriteTime;
        seqWrite.latency = (seqWriteTime / NUM_OPS) * 1000.0;
        seqWrite.iops = NUM_OPS / seqWriteTime;
    }

    report.addResult(seqWrite);
    seqWrite.display();

    // =========================
    // Sequential Read
    // =========================
    double seqReadTime = disk.sequentialRead(NUM_OPS);

    IOMetrics seqRead;
    seqRead.testName = "Seq Read";
    seqRead.totalBytes = NUM_OPS * disk.getSectorSize();
    seqRead.totalOperations = NUM_OPS;
    seqRead.duration = seqReadTime;

    if (seqReadTime > 0) {
        seqRead.throughput = seqRead.totalBytes / seqReadTime;
        seqRead.latency = (seqReadTime / NUM_OPS) * 1000.0;
        seqRead.iops = NUM_OPS / seqReadTime;
    }

    report.addResult(seqRead);
    seqRead.display();

    // =========================
    // Random Write
    // =========================
    double randWriteTime = disk.randomWrite(NUM_OPS);

    IOMetrics randWrite;
    randWrite.testName = "Rand Write";
    randWrite.totalBytes = NUM_OPS * disk.getSectorSize();
    randWrite.totalOperations = NUM_OPS;
    randWrite.duration = randWriteTime;

    if (randWriteTime > 0) {
        randWrite.throughput = randWrite.totalBytes / randWriteTime;
        randWrite.latency = (randWriteTime / NUM_OPS) * 1000.0;
        randWrite.iops = NUM_OPS / randWriteTime;
    }

    report.addResult(randWrite);
    randWrite.display();

    // =========================
    // Random Read
    // =========================
    double randReadTime = disk.randomRead(NUM_OPS);

    IOMetrics randRead;
    randRead.testName = "Rand Read";
    randRead.totalBytes = NUM_OPS * disk.getSectorSize();
    randRead.totalOperations = NUM_OPS;
    randRead.duration = randReadTime;

    if (randReadTime > 0) {
        randRead.throughput = randRead.totalBytes / randReadTime;
        randRead.latency = (randReadTime / NUM_OPS) * 1000.0;
        randRead.iops = NUM_OPS / randReadTime;
    }

    report.addResult(randRead);
    randRead.display();

    // =========================
    // Summary Table
    // =========================
    report.generateReport();

    return 0;
}