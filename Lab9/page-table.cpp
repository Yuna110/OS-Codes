/*
 * Exercise 1: Page Table Simulator
 * This program simulates a simple page table with virtual to physical address translation
 */

#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;

// Page Table Entry class
class PageTableEntry {
private:
    int frameNumber;      // Physical frame number
    bool valid;           // Valid bit (is page in memory?)
    bool dirty;           // Dirty bit (has page been modified?)
    bool referenced;      // Reference bit (has page been accessed?)

public:
    // Constructor
    PageTableEntry() : frameNumber(-1), valid(false), dirty(false), referenced(false) {}
    
    // Getters
    int getFrameNumber() const { return frameNumber; }
    bool isValid() const { return valid; }
    bool isDirty() const { return dirty; }
    bool isReferenced() const { return referenced; }
    
    // Setters
    void setFrameNumber(int frame) { frameNumber = frame; }
    void setValid(bool v) { valid = v; }
    void setDirty(bool d) { dirty = d; }
    void setReferenced(bool r) { referenced = r; }
    
    // Display entry
    void display() const {
        cout << "Frame: " << setw(3) << frameNumber 
             << " | Valid: " << valid 
             << " | Dirty: " << dirty 
             << " | Referenced: " << referenced;
    }
};

// Page Table class
class PageTable {
private:
    vector<PageTableEntry> entries;
    int pageSize;         // Size of each page in bytes
    int numPages;         // Number of pages in virtual address space
    
public:
    // Constructor
    PageTable(int numPages, int pageSize) : numPages(numPages), pageSize(pageSize) {
        entries.resize(numPages);
    }
    
    // Add a page mapping
    void addMapping(int pageNumber, int frameNumber) {
        if (pageNumber >= 0 && pageNumber < numPages) {
            entries[pageNumber].setFrameNumber(frameNumber);
            entries[pageNumber].setValid(true);
        }
    }
    
    // Remove a page mapping
    void removeMapping(int pageNumber) {
        if (pageNumber >= 0 && pageNumber < numPages) {
            entries[pageNumber].setValid(false);
        }
    }
    
    // Translate virtual address to physical address
    int translate(int virtualAddress) {
        int pageNumber = virtualAddress / pageSize;
        int offset = virtualAddress % pageSize;
        
        cout << "\nTranslating Virtual Address: " << virtualAddress << endl;
        cout << "Page Number: " << pageNumber << ", Offset: " << offset << endl;
        
        if (pageNumber < 0 || pageNumber >= numPages) {
            cout << "ERROR: Invalid page number!" << endl;
            return -1;
        }
        
        if (!entries[pageNumber].isValid()) {
            cout << "PAGE FAULT: Page " << pageNumber << " not in memory!" << endl;
            return -1;
        }
        
        // Mark as referenced
        entries[pageNumber].setReferenced(true);
        
        int frameNumber = entries[pageNumber].getFrameNumber();
        int physicalAddress = frameNumber * pageSize + offset;
        
        cout << "Physical Address: " << physicalAddress << endl;
        return physicalAddress;
    }
    
    // Write to an address (sets dirty bit)
    void write(int virtualAddress) {
        int pageNumber = virtualAddress / pageSize;
        
        if (pageNumber >= 0 && pageNumber < numPages && entries[pageNumber].isValid()) {
            entries[pageNumber].setDirty(true);
            entries[pageNumber].setReferenced(true);
            cout << "Write successful. Page " << pageNumber << " marked as dirty." << endl;
        }
    }
    
    // Display entire page table
    void display() const {
        cout << "\n====== PAGE TABLE ======" << endl;
        cout << "Page Size: " << pageSize << " bytes" << endl;
        cout << "Number of Pages: " << numPages << endl;
        cout << "\nPage# | ";
        PageTableEntry temp;
        temp.display();
        cout << endl;
        cout << string(70, '-') << endl;
        
        for (int i = 0; i < numPages; i++) {
            if (entries[i].isValid()) {
                cout << setw(5) << i << " | ";
                entries[i].display();
                cout << endl;
            }
        }
        cout << "========================" << endl;
    }
};

int main() {
    cout << "=== Page Table Simulator ===" << endl;
    
    // Create a page table with 16 pages, each 512 bytes
    PageTable pt(16, 512);
    
    // Add some page mappings (page -> frame)
    pt.addMapping(0, 5);    // Page 0 maps to Frame 5
    pt.addMapping(1, 2);    // Page 1 maps to Frame 2
    pt.addMapping(2, 10);   // Page 2 maps to Frame 10
    pt.addMapping(3, 7);    // Page 3 maps to Frame 7
    pt.addMapping(5, 3);    // Page 5 maps to Frame 3
    
    // Display initial page table
    pt.display();
    
    // Test translations
    cout << "\n=== Testing Address Translations ===" << endl;
    
    // Test 1: Valid translation
    pt.translate(1024);     // Page 2, Offset 0
    
    // Test 2: Another valid translation
    pt.translate(256);      // Page 0, Offset 256
    
    // Test 3: Page fault
    pt.translate(2048);     // Page 4, not in memory
    
    // Test 4: Write operation
    cout << "\n=== Testing Write Operation ===" << endl;
    pt.write(512);          // Write to Page 1
    
    // Display updated page table
    pt.display();
    
    // Test 5: More translations
    cout << "\n=== More Translations ===" << endl;
    pt.translate(2560);     // Page 5, Offset 0
    pt.translate(1800);     // Page 3, Offset 264
    
    // Final page table state
    pt.display();
    
    return 0;
}
