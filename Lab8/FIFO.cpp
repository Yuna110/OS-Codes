#include <iostream>
#include <queue>
#include <unordered_set>
#include <vector>
#include <iomanip>

using namespace std;

class FIFOPageReplacement {
private:
    int frames;
    queue<int> fifoQueue;
    unordered_set<int> resident;
    int pageFaults = 0;

    vector<vector<int>> history;   // Frame contents per step

    void recordState() {
        vector<int> snapshot;
        queue<int> temp = fifoQueue;

        while (!temp.empty()) {
            snapshot.push_back(temp.front());
            temp.pop();
        }
        history.push_back(snapshot);
    }

public:
    FIFOPageReplacement(int f) : frames(f) {}

    void reset() {
        pageFaults = 0;
        history.clear();
        fifoQueue = queue<int>();
        resident.clear();
    }

    void access(int page) {
        cout << "\nReferencing page " << page << endl;

        if (resident.count(page)) {
            cout << " HIT\n";
        } else {
            pageFaults++;
            cout << " PAGE FAULT #" << pageFaults << endl;

            if ((int)fifoQueue.size() == frames) {
                int victim = fifoQueue.front();
                fifoQueue.pop();
                resident.erase(victim);
                cout << " Evicted page " << victim << endl;
            }

            fifoQueue.push(page);
            resident.insert(page);
            cout << " Loaded page " << page << endl;
        }

        recordState();
    }

    void simulate(const vector<int>& refs) {
        reset();

        cout << "\n========== FIFO PAGE REPLACEMENT ==========\n";
        cout << "Frames: " << frames << endl;
        cout << "Reference String: ";
        for (int p : refs) cout << p << " ";
        cout << "\n";

        for (int p : refs)
            access(p);

        displayStats();
        displayTable(refs);
    }

    void displayStats() const {
        int total = history.size();
        double faultRate = total ? (100.0 * pageFaults / total) : 0.0;

        cout << "\n=== RESULTS ===\n";
        cout << "Total References : " << total << endl;
        cout << "Page Faults      : " << pageFaults << endl;
        cout << "Fault Rate       : "
             << fixed << setprecision(2) << faultRate << "%\n";
        cout << "Hit Rate         : "
             << fixed << setprecision(2) << (100.0 - faultRate) << "%\n";
    }

    void displayTable(const vector<int>& refs) const {
        cout << "\n=== FRAME CONTENT TABLE ===\n";

        cout << setw(8) << "Frame |";
        for (int p : refs)
            cout << setw(4) << p;
        cout << "\n" << string(8 + refs.size() * 4 + 1, '-') << endl;

        for (int f = 0; f < frames; f++) {
            cout << setw(6) << ("F" + to_string(f)) << " |";
            for (const auto& step : history) {
                if (f < (int)step.size())
                    cout << setw(4) << step[f];
                else
                    cout << setw(4) << "-";
            }
            cout << endl;
        }
    }
};

/* =========================
   Main Driver
   ========================= */
int main() {
    cout << "PAGE REPLACEMENT - FIFO\n";
    cout << "=======================\n";

    // Test case 1
    cout << "\n--- TEST CASE 1 ---";
    FIFOPageReplacement fifo1(3);
    vector<int> ref1 = {7,0,1,2,0,3,0,4,2,3,0,3,2};
    fifo1.simulate(ref1);

    // Test case 2
    cout << "\n--- TEST CASE 2 ---";
    FIFOPageReplacement fifo2(4);
    vector<int> ref2 = {1,2,3,4,1,2,5,1,2,3,4,5};
    fifo2.simulate(ref2);

    // Custom input
    char choice;
    cout << "\nRun custom test? (y/n): ";
    cin >> choice;

    if (choice == 'y' || choice == 'Y') {
        int f;
        cout << "Number of frames: ";
        cin >> f;

        FIFOPageReplacement custom(f);
        cout << "Enter reference string (-1 to end): ";

        vector<int> refs;
        int p;
        while (cin >> p && p != -1)
            refs.push_back(p);

        custom.simulate(refs);
    }

    return 0;
}
