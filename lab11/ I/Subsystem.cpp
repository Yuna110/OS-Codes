#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <map>
#include <mutex>

using namespace std;

// ============================================
// IO Request Structure
// ============================================
struct IORequest {
    enum class Type { READ, WRITE };
    enum class Priority { HIGH, MEDIUM, LOW };

    int requestId;
    Type type;
    Priority priority;
    size_t blockNumber;
    size_t dataSize;
    vector<uint8_t> data;
    chrono::time_point<chrono::steady_clock> arrivalTime;

    IORequest(int id, Type t, Priority p, size_t block, size_t size)
        : requestId(id), type(t), priority(p),
          blockNumber(block), dataSize(size),
          data(size, 0),
          arrivalTime(chrono::steady_clock::now()) {}

    string getTypeStr() const {
        return type == Type::READ ? "READ" : "WRITE";
    }

    string getPriorityStr() const {
        switch (priority) {
            case Priority::HIGH: return "HIGH";
            case Priority::MEDIUM: return "MEDIUM";
            case Priority::LOW: return "LOW";
        }
        return "UNKNOWN";
    }

    void display() const {
        cout << "Request[" << requestId << "] "
             << getTypeStr()
             << " Block:" << blockNumber
             << " Size:" << dataSize
             << " Priority:" << getPriorityStr()
             << endl;
    }
};

// ============================================
// FCFS Scheduler
// ============================================
class FCFSScheduler {
private:
    queue<IORequest> requestQueue;
    int processedCount = 0;

public:
    void addRequest(const IORequest& req) {
        requestQueue.push(req);
    }

    bool processNext() {
        if (requestQueue.empty()) return false;

        IORequest req = requestQueue.front();
        requestQueue.pop();

        cout << "[FCFS] Processing: ";
        req.display();

        processedCount++;
        return true;
    }

    void processAll() {
        while (processNext()) {}
        cout << "[FCFS] Total processed: "
             << processedCount << endl;
    }
};

// ============================================
// Priority Scheduler
// ============================================
class PriorityScheduler {
private:
    queue<IORequest> highQ, medQ, lowQ;
    int processedCount = 0;

public:
    void addRequest(const IORequest& req) {
        switch (req.priority) {
            case IORequest::Priority::HIGH: highQ.push(req); break;
            case IORequest::Priority::MEDIUM: medQ.push(req); break;
            case IORequest::Priority::LOW: lowQ.push(req); break;
        }
    }

    bool processNext() {
        IORequest req(0, IORequest::Type::READ,
                      IORequest::Priority::LOW, 0, 0);

        if (!highQ.empty()) {
            req = highQ.front(); highQ.pop();
        }
        else if (!medQ.empty()) {
            req = medQ.front(); medQ.pop();
        }
        else if (!lowQ.empty()) {
            req = lowQ.front(); lowQ.pop();
        }
        else return false;

        cout << "[PRIORITY] Processing: ";
        req.display();

        processedCount++;
        return true;
    }

    void processAll() {
        while (processNext()) {}
        cout << "[PRIORITY] Total processed: "
             << processedCount << endl;
    }
};

// ============================================
// Circular Buffer
// ============================================
class CircularBuffer {
private:
    vector<uint8_t> buffer;
    size_t head = 0, tail = 0, count = 0;
    size_t capacity;
    mutex mtx;

public:
    CircularBuffer(size_t size)
        : buffer(size), capacity(size) {}

    bool write(uint8_t value) {
        lock_guard<mutex> lock(mtx);

        if (count == capacity)
            return false;

        buffer[tail] = value;
        tail = (tail + 1) % capacity;
        count++;
        return true;
    }

    bool read(uint8_t& value) {
        lock_guard<mutex> lock(mtx);

        if (count == 0)
            return false;

        value = buffer[head];
        head = (head + 1) % capacity;
        count--;
        return true;
    }
};

// ============================================
// IO Cache with LRU
// ============================================
class IOCache {
private:
    struct CacheEntry {
        vector<uint8_t> data;
        bool dirty;
        chrono::time_point<chrono::steady_clock> lastAccess;

        CacheEntry(const vector<uint8_t>& d)
            : data(d), dirty(false),
              lastAccess(chrono::steady_clock::now()) {}
    };

    map<size_t, CacheEntry> cache;
    size_t maxEntries;
    int hits = 0, misses = 0;

public:
    IOCache(size_t maxSize)
        : maxEntries(maxSize) {}

    bool lookup(size_t blockNum, vector<uint8_t>& outData) {
        auto it = cache.find(blockNum);

        if (it == cache.end()) {
            misses++;
            return false;
        }

        hits++;
        it->second.lastAccess = chrono::steady_clock::now();
        outData = it->second.data;
        return true;
    }

    void insert(size_t blockNum, const vector<uint8_t>& data) {
        if (cache.size() >= maxEntries)
            evict();

        cache.emplace(blockNum, CacheEntry(data));
    }

    void evict() {
        if (cache.empty()) return;

        auto lru = cache.begin();
        for (auto it = cache.begin(); it != cache.end(); ++it) {
            if (it->second.lastAccess <
                lru->second.lastAccess) {
                lru = it;
            }
        }

        cache.erase(lru);
    }

    double hitRate() const {
        int total = hits + misses;
        return total == 0 ? 0.0 :
               (double)hits / total * 100.0;
    }
};

// ============================================
// MAIN
// ============================================
int main() {
    cout << "=== Kernel I/O Subsystem Simulation ===\n";

    vector<IORequest> requests = {
        {1, IORequest::Type::READ,  IORequest::Priority::LOW, 5, 512},
        {2, IORequest::Type::WRITE, IORequest::Priority::HIGH, 1, 512},
        {3, IORequest::Type::READ,  IORequest::Priority::MEDIUM, 3, 1024},
    };

    FCFSScheduler fcfs;
    for (auto& r : requests) fcfs.addRequest(r);
    fcfs.processAll();

    PriorityScheduler ps;
    for (auto& r : requests) ps.addRequest(r);
    ps.processAll();

    CircularBuffer cb(4);
    cb.write(10);
    cb.write(20);

    uint8_t val;
    while (cb.read(val))
        cout << "Read: " << (int)val << endl;

    IOCache cache(2);
    vector<uint8_t> block(512, 0xAA), out;
    cache.lookup(1, out);
    cache.insert(1, block);
    cache.lookup(1, out);

    cout << "Cache Hit Rate: "
         << cache.hitRate() << "%\n";

    return 0;
}
