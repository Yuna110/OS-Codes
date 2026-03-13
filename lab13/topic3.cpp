#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <iomanip>
#include <ctime>

using namespace std;

// ── Port Scanner (attacker view) ─────────────────────────────
struct PortInfo {
    string service;
    bool vulnerable;
    string cve;
};

class PortScanner {
    map<int, PortInfo> portDB = {
        {21, {"FTP", true, "CVE-2010-4221"}},
        {22, {"SSH", false, ""}},
        {23, {"Telnet", true, "CVE-2011-4862"}},
        {80, {"HTTP", false, ""}},
        {443, {"HTTPS", false, ""}},
        {445, {"SMB", true, "CVE-2017-0144"}}, // EternalBlue
        {3306, {"MySQL", true, "CVE-2012-2122"}},
        {4444, {"Backdoor/mshell", true, "N/A"}}
    };

public:
    void scan(const string& target, int lo, int hi) {
        cout << "[SCAN] Target: " << target
             << " Range: " << lo << "-" << hi << "\n\n";

        cout << left
             << setw(6) << "PORT"
             << setw(10) << "STATE"
             << setw(18) << "SERVICE"
             << "CVE\n"
             << string(56, '-') << "\n";

        for (int p = lo; p <= hi; p++) {
            auto it = portDB.find(p);

            if (it != portDB.end()) {
                cout << setw(6) << p
                     << setw(10) << "OPEN"
                     << setw(18) << it->second.service;

                if (it->second.vulnerable)
                    cout << "[VULN] " << it->second.cve;
                else
                    cout << "OK";

                cout << "\n";
            }
        }
    }
};

// ── IDS (Snort-style defender view) ──────────────────────────
class IDS {

    struct Attempt {
        string ip;
        int port;
        time_t ts;
    };

    vector<Attempt> log_;

    const int THRESHOLD = 5;
    const int WINDOW = 3; // seconds

public:
    void observe(const string& ip, int port) {

        log_.push_back({ip, port, time(nullptr)});

        time_t now = time(nullptr);
        int count = 0;

        for (auto& e : log_)
            if (e.ip == ip && (now - e.ts) <= WINDOW)
                count++;

        if (count == THRESHOLD + 1) { // alert once
            cout << "\n[IDS ALERT] Port scan from " << ip
                 << " (" << count << " probes in "
                 << WINDOW << "s)\n";

            cout << "[IDS ACTION] Simulating: "
                 << "iptables -I INPUT -s " << ip << " -j DROP\n\n";
        }
    }

    void printLog() {
        cout << "\n[IDS LOG]\n";

        for (auto& e : log_)
            cout << " " << e.ip << " -> :" << e.port
                 << " @ t+" << (e.ts - log_[0].ts) << "s\n";
    }
};

// ── Worm Propagation ─────────────────────────────────────────
class Worm {

    vector<string> infected_;

    const int RATE = 3; // new hosts per infected host per iteration

public:
    void spread(const string& seed, int rounds) {

        infected_.push_back(seed);

        cout << "[WORM] Seed host: " << seed << "\n";

        for (int r = 0; r < rounds; r++) {

            size_t prev = infected_.size();
            size_t lim = prev;

            for (size_t i = 0; i < lim; i++) {

                for (int k = 0; k < RATE; k++) {

                    string h = "10." + to_string(r + 1) + "."
                             + to_string((int)i) + "."
                             + to_string(k + 2);

                    if (find(infected_.begin(),
                             infected_.end(), h) == infected_.end())
                        infected_.push_back(h);
                }
            }

            cout << "[WORM] Round " << r + 1
                 << ": +" << (infected_.size() - prev)
                 << " new hosts | total=" << infected_.size() << "\n";
        }
    }
};

int main() {

    cout << "=== System & Network Threats Demo ===\n\n";

    // Port scan
    cout << "─── Attacker: Reconnaissance ───\n";
    PortScanner ps;
    ps.scan("192.168.1.10", 20, 4450);

    // IDS
    cout << "\n─── Defender: IDS Monitoring ───\n";

    IDS ids;
    string atk = "203.0.113.99";

    for (int p : {21, 22, 23, 80, 443, 445, 3306, 8080})
        ids.observe(atk, p);

    ids.printLog();

    // Worm
    cout << "\n─── Worm Propagation ───\n";

    Worm worm;
    worm.spread("192.168.1.1", 4);

    return 0;
}