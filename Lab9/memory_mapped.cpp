/*
 * Exercise 6: Memory-Mapped File I/O Simulation
 * Demonstrates memory-mapped file concepts with demand paging
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cmath>

using namespace std;

class MemoryMappedFile {
private:
    string filename;
    vector<char> fileContent;       // Simulated file content
    int fileSize;                   // File size in bytes
    int pageSize;                   // Page size in bytes
    int numPages;                   // Number of pages
    
    map<int, bool> pagesInMemory;   // Track which pages are loaded
    vector<vector<char>> pages;     // Actual page content
    
    int pageLoads;                  // Statistics
    int reads;
    int totalReadTime;              // Simulated time in microseconds
    
    const int DISK_ACCESS_TIME = 1000;  // 1ms in microseconds
    const int MEMORY_ACCESS_TIME = 1;    // 1 microsecond
    
    // Load a page from "disk" to memory
    void loadPage(int pageNumber) {
        if (pageNumber < 0 || pageNumber >= numPages) {
            return;
        }
        
        if (pagesInMemory[pageNumber]) {
            return;  // Already loaded
        }
        
        cout << "  Loading page " << pageNumber << " from disk..." << endl;
        
        int startByte = pageNumber * pageSize;
        int endByte = min(startByte + pageSize, fileSize);
        
        vector<char> pageContent;
        for (int i = startByte; i < endByte; i++) {
            pageContent.push_back(fileContent[i]);
        }
        
        pages[pageNumber] = pageContent;
        pagesInMemory[pageNumber] = true;
        pageLoads++;
        totalReadTime += DISK_ACCESS_TIME;
    }
    
public:
    // Constructor
    MemoryMappedFile(const string& file, int pgSize = 4096) 
        : filename(file), pageSize(pgSize), pageLoads(0), reads(0), totalReadTime(0) {
    }
    
    // Initialize file (create or load)
    bool initialize(const string& content = "") {
        if (!content.empty()) {
            // Create file with content
            fileContent = vector<char>(content.begin(), content.end());
        } else {
            // Try to load existing file
            ifstream file(filename, ios::binary);
            if (file) {
                file.seekg(0, ios::end);
                fileSize = file.tellg();
                file.seekg(0, ios::beg);
                
                fileContent.resize(fileSize);
                file.read(&fileContent[0], fileSize);
                file.close();
            } else {
                // Create sample file
                string sampleContent = 
                    "This is a sample file for memory-mapped I/O demonstration. "
                    "Memory-mapped files allow treating file content as if it were in memory. "
                    "This provides efficient file access through demand paging. "
                    "Pages are loaded only when accessed, reducing initial overhead. "
                    "This is particularly useful for large files where only portions are needed. "
                    "The operating system handles the complexity of loading and unloading pages. "
                    "This technique is commonly used in databases, executables, and shared libraries. ";
                
                fileContent = vector<char>(sampleContent.begin(), sampleContent.end());
            }
        }
        
        fileSize = fileContent.size();
        numPages = (fileSize + pageSize - 1) / pageSize;  // Ceiling division
        
        pages.resize(numPages);
        
        cout << "File initialized: " << filename << endl;
        cout << "File size: " << fileSize << " bytes" << endl;
        cout << "Page size: " << pageSize << " bytes" << endl;
        cout << "Number of pages: " << numPages << endl;
        
        return true;
    }
    
    // Read data at offset
    string read(int offset, int length) {
        reads++;
        
        cout << "\nRead request: Offset=" << offset << ", Length=" << length << endl;
        
        if (offset < 0 || offset >= fileSize) {
            cout << "  Error: Invalid offset" << endl;
            return "";
        }
        
        length = min(length, fileSize - offset);
        
        int startPage = offset / pageSize;
        int endPage = (offset + length - 1) / pageSize;
        
        cout << "  Pages needed: " << startPage << " to " << endPage << endl;
        
        // Load required pages
        for (int p = startPage; p <= endPage; p++) {
            loadPage(p);
            totalReadTime += MEMORY_ACCESS_TIME;
        }
        
        // Extract data
        string result;
        for (int i = offset; i < offset + length; i++) {
            result += fileContent[i];
        }
        
        return result;
    }
    
    // Display memory status
    void displayMemoryStatus() {
        cout << "\n=== Memory Status ===" << endl;
        cout << "Pages in memory: ";
        
        int loadedPages = 0;
        for (int i = 0; i < numPages; i++) {
            if (pagesInMemory[i]) {
                cout << i << " ";
                loadedPages++;
            }
        }
        
        cout << "\nTotal loaded: " << loadedPages << "/" << numPages 
             << " (" << (loadedPages * 100.0 / numPages) << "%)" << endl;
    }
    
    // Display statistics
    void displayStatistics() {
        cout << "\n=== Statistics ===" << endl;
        cout << "Total reads: " << reads << endl;
        cout << "Page loads: " << pageLoads << endl;
        cout << "Pages per read: " << fixed << setprecision(2) 
             << (reads > 0 ? (double)pageLoads / reads : 0) << endl;
        cout << "Total access time: " << totalReadTime << " Î¼s" << endl;
        cout << "Average access time: " << fixed << setprecision(2)
             << (reads > 0 ? (double)totalReadTime / reads : 0) << " Î¼s" << endl;
    }
    
    // Compare with traditional I/O
    void compareWithTraditionalIO() {
        cout << "\n=== Comparison with Traditional I/O ===" << endl;
        
        // For traditional I/O, each read would require full file access
        int traditionalTime = reads * DISK_ACCESS_TIME;
        
        cout << "Memory-Mapped I/O time: " << totalReadTime << " Î¼s" << endl;
        cout << "Traditional I/O time (estimated): " << traditionalTime << " Î¼s" << endl;
        
        if (totalReadTime < traditionalTime) {
            double speedup = (double)traditionalTime / totalReadTime;
            cout << "Speedup: " << fixed << setprecision(2) << speedup << "x faster" << endl;
        }
    }
};

// Simulate traditional file I/O for comparison
class TraditionalFileIO {
private:
    string filename;
    vector<char> fileContent;
    int fileSize;
    int reads;
    int totalReadTime;
    const int DISK_ACCESS_TIME = 1000;
    
public:
    TraditionalFileIO(const string& file) : filename(file), reads(0), totalReadTime(0) {}
    
    void initialize(const vector<char>& content) {
        fileContent = content;
        fileSize = content.size();
    }
    
    string read(int offset, int length) {
        reads++;
        
        // Each read requires disk access
        totalReadTime += DISK_ACCESS_TIME;
        
        if (offset < 0 || offset >= fileSize) {
            return "";
        }
        
        length = min(length, fileSize - offset);
        string result;
        for (int i = offset; i < offset + length; i++) {
            result += fileContent[i];
        }
        
        return result;
    }
    
    int getTotalTime() const { return totalReadTime; }
};

int main() {
    cout << "=== Memory-Mapped File I/O Simulator ===" << endl;
    
    // Test case 1: Basic memory-mapped file operations
    cout << "\n*** Test 1: Basic Operations ***" << endl;
    
    MemoryMappedFile mmf("sample.txt", 100);  // 100-byte pages
    
    string content = 
        "Memory-mapped files provide an efficient way to access file data. "
        "The file is divided into pages and loaded on demand. "
        "This reduces memory usage and improves performance for large files. "
        "Only the pages that are actually accessed need to be loaded. "
        "This is similar to how virtual memory works for program code and data.";
    
    mmf.initialize(content);
    
    // Read different portions
    cout << "\n--- Reading small portion ---" << endl;
    string data1 = mmf.read(0, 50);
    cout << "Data: \"" << data1 << "\"" << endl;
    
    mmf.displayMemoryStatus();
    
    cout << "\n--- Reading from middle ---" << endl;
    string data2 = mmf.read(150, 60);
    cout << "Data: \"" << data2 << "\"" << endl;
    
    mmf.displayMemoryStatus();
    
    cout << "\n--- Reading previously accessed area ---" << endl;
    string data3 = mmf.read(20, 30);
    cout << "Data: \"" << data3 << "\"" << endl;
    
    mmf.displayMemoryStatus();
    mmf.displayStatistics();
    
    // Test case 2: Compare with traditional I/O
    cout << "\n\n*** Test 2: Performance Comparison ***" << endl;
    
    MemoryMappedFile mmf2("large_file.txt", 1024);
    
    string largeContent;
    for (int i = 0; i < 100; i++) {
        largeContent += "This is line " + to_string(i) + " of the large file. "
                       "It contains some data for demonstration purposes. ";
    }
    
    mmf2.initialize(largeContent);
    
    // Perform multiple reads with memory-mapped I/O
    cout << "\nPerforming multiple reads with Memory-Mapped I/O:" << endl;
    mmf2.read(0, 100);
    mmf2.read(500, 100);
    mmf2.read(1000, 100);
    mmf2.read(50, 50);      // Overlaps with first read
    mmf2.read(2000, 100);
    
    mmf2.displayMemoryStatus();
    mmf2.displayStatistics();
    mmf2.compareWithTraditionalIO();
    
    // Test case 3: Demonstrate locality benefits
    cout << "\n\n*** Test 3: Locality of Reference ***" << endl;
    
    MemoryMappedFile mmf3("locality_test.txt", 512);
    
    string testContent(10000, 'X');  // Large file
    mmf3.initialize(testContent);
    
    cout << "\nScenario 1: Sequential access (good locality)" << endl;
    for (int i = 0; i < 5; i++) {
        mmf3.read(i * 100, 50);
    }
    mmf3.displayMemoryStatus();
    mmf3.displayStatistics();
    
    // Test case 4: Random access pattern
    cout << "\n\n*** Test 4: Random Access Pattern ***" << endl;
    
    MemoryMappedFile mmf4("random_access.txt", 256);
    mmf4.initialize(string(5000, 'Y'));
    
    cout << "\nRandom access pattern (poor locality):" << endl;
    int offsets[] = {0, 1000, 500, 3000, 100, 2000, 4000};
    for (int offset : offsets) {
        mmf4.read(offset, 50);
    }
    
    mmf4.displayMemoryStatus();
    mmf4.displayStatistics();
    
    return 0;
}
