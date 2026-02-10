#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
using namespace std;

const int NUM_PAGES  = 64;
const int NUM_FRAMES = 32;
const int PAGE_SIZE  = 1024;
const int OFFSET_BITS = 10;   // log2(1024)
const int PAGE_BITS   = 6;    // log2(64)

class PageTable {
private:
    int  pageTable[NUM_PAGES];
    bool valid[NUM_PAGES];

public:
    PageTable() {
        srand(time(0));

        bool usedFrames[NUM_FRAMES] = {false};
        int framesUsed = 0;

        for (int i = 0; i < NUM_PAGES; i++) {

            // Only assign a frame if one is still available
            if ((rand() % 100 < 75) && framesUsed < NUM_FRAMES) {

                int frame;
                do {
                    frame = rand() % NUM_FRAMES;
                } while (usedFrames[frame]);

                pageTable[i] = frame;
                valid[i] = true;
                usedFrames[frame] = true;
                framesUsed++;

            } else {
                pageTable[i] = -1;
                valid[i] = false;
            }
        }
    }

    int translateAddress(int logicalAddress) {

        // Extract page number and offset using bit operations
        int pageNumber = logicalAddress >> OFFSET_BITS;
        int offset     = logicalAddress & (PAGE_SIZE - 1);

        if (pageNumber < 0 || pageNumber >= NUM_PAGES) {
            cout << "Error: Invalid page number " << pageNumber << endl;
            return -1;
        }

        if (!valid[pageNumber]) {
            cout << "Page Fault: Page " << pageNumber << " is not in memory" << endl;
            return -1;
        }

        int frameNumber = pageTable[pageNumber];
        int physicalAddress = (frameNumber << OFFSET_BITS) | offset;

        cout << "Logical Address : " << logicalAddress << endl;
        cout << " Page Number    : " << pageNumber << endl;
        cout << " Offset         : " << offset << endl;
        cout << " Frame Number   : " << frameNumber << endl;
        cout << "Physical Address: " << physicalAddress << endl;

        return physicalAddress;
    }

    void displayPageTable() {
        cout << "\n=== PAGE TABLE ===\n";
        cout << setw(10) << "Page#"
             << setw(10) << "Frame#"
             << setw(10) << "Valid\n";
        cout << string(30, '-') << endl;

        for (int i = 0; i < NUM_PAGES; i++) {
            cout << setw(10) << i
                 << setw(10) << (valid[i] ? to_string(pageTable[i]) : "N/A")
                 << setw(10) << (valid[i] ? "Yes" : "No") << endl;
        }
    }
};

int main() {

    cout << "ADDRESS TRANSLATION SIMULATOR\n";
    cout << "=============================\n";
    cout << "Page Size       : " << PAGE_SIZE << " bytes\n";
    cout << "Number of Pages : " << NUM_PAGES << endl;
    cout << "Number of Frames: " << NUM_FRAMES << "\n\n";

    PageTable pt;

    int testAddresses[] = {0, 1024, 2048, 5120, 10240, 65535};

    for (int addr : testAddresses) {
        cout << "\n--- Translation Test ---\n";
        pt.translateAddress(addr);
    }

    pt.displayPageTable();

    char choice;
    cout << "\nEnter addresses interactively? (y/n): ";
    cin >> choice;

    if (choice == 'y' || choice == 'Y') {
        int addr;
        while (true) {
            cout << "\nEnter logical address (-1 to quit): ";
            cin >> addr;
            if (addr < 0) break;
            pt.translateAddress(addr);
        }
    }

    return 0;
}
