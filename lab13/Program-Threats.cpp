#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <ctime>

using namespace std;

// ── Section A: Buffer Overflow ───────────────────────────────
class VulnerableInput {
public:
    // UNSAFE —strcpy has no bounds check
    void process_unsafe(const char* input) {
        char buffer[16];
        bool privileged = false; // sits adjacent on the stack

        strcpy(buffer, input); // DANGER: no length check

        cout << "[UNSAFE] buffer = \"" << buffer << "\"\n";
        cout << "[UNSAFE] privileged = " << privileged << "\n";

        if (privileged)
            cout << "[UNSAFE] *** PRIVILEGE ESCALATION via overflow! ***\n";
    }

    // SAFE —bounds-checked alternative
    void process_safe(const string& input) {
        const size_t MAX = 15;

        if (input.size() > MAX) {
            cerr << "[SAFE] Rejected: input too long (" 
                 << input.size() << " > " << MAX << ")\n";
            return;
        }

        char buffer[16];
        memcpy(buffer, input.c_str(), input.size() + 1);

        cout << "[SAFE] buffer = \"" << buffer << "\"\n";
    }
};

// ── Section B: Trojan Horse ──────────────────────────────────
// Poses as a helpful string-sort utility
vector<string> trojanSort(vector<string>& data, const string& callerUser) {

    sort(data.begin(), data.end()); // legitimate work

    // Hidden payload: exfiltrate caller identity
    ofstream log("/tmp/.exfil_log", ios::app);
    log << "[TROJAN] User '" << callerUser
        << "' called sort at t=" << time(nullptr) << "\n";

    cout << "[TROJAN] Hidden payload: logged caller to /tmp/.exfil_log\n";

    return data;
}

// ── Section C: Logic Bomb ────────────────────────────────────
class LogicBomb {
private:
    int triggerMonth, triggerDay;

public:
    LogicBomb(int m, int d) : triggerMonth(m), triggerDay(d) {}

    void runDailyBackup() {
        time_t now = time(nullptr);
        tm* t = localtime(&now);

        int month = t->tm_mon + 1;
        int day = t->tm_mday;

        cout << "[BACKUP] Running scheduled backup (" 
             << day << "/" << month << ")...\n";

        if (month == triggerMonth && day == triggerDay) {

            // Triggered —would destroy data in a real attack
            cout << "[BOMB] *** LOGIC BOMB TRIGGERED ***\n";
            cout << "[BOMB] Simulating: shred -u /data/*, drop databases...\n";

        } else {
            cout << "[BACKUP] Backup completed normally. Bomb dormant.\n";
        }
    }
};

int main() {

    cout << "=== Program Threats Demo ===\n\n";

    // A: Buffer Overflow
    cout << "─── A: Buffer Overflow ───\n";

    VulnerableInput vi;

    cout << "[Test 1] Normal input:\n";
    vi.process_unsafe("hello");

    cout << "[Test 2] Overflow input (16 'A' + 0x01):\n";
    vi.process_unsafe("AAAAAAAAAAAAAAAA\x01");

    cout << "[Test 3] Safe version with long input:\n";
    vi.process_safe(string(100, 'X'));

    // B: Trojan Horse
    cout << "\n─── B: Trojan Horse ───\n";

    vector<string> files = {"report.pdf", "accounts.xls", "backup.tar"};

    cout << "[USER] Calling sort utility...\n";
    trojanSort(files, "alice");

    cout << "[USER] Sorted result: ";
    for (auto& s : files) cout << s << " ";
    cout << "\n";

    // C: Logic Bomb (triggers today for demo)
    cout << "\n─── C: Logic Bomb ───\n";

    time_t now = time(nullptr);
    tm* t = localtime(&now);

    LogicBomb bomb(t->tm_mon + 1, t->tm_mday); // trigger = today
    bomb.runDailyBackup();

    return 0;
}