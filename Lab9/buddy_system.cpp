/*
 * Exercise 7: Buddy System Memory Allocator
 * Implements the buddy system for kernel memory allocation
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

using namespace std;

struct Block {
    int size;           // Size of block (power of 2)
    int address;        // Starting address
    bool allocated;     // Is block allocated?
    int id;            // Allocation ID
    
    Block(int s, int a) : size(s), address(a), allocated(false), id(-1) {}
};

class BuddySystem {
private:
    int totalMemory;                    // Total memory size
    int minBlockSize;                   // Minimum block size
    map<int, vector<Block>> freeList;   // Free blocks by size
    vector<Block> allocatedBlocks;      // Allocated blocks
    int nextId;                         // Next allocation ID
    
    // Calculate next power of 2 >= n
    int nextPowerOf2(int n) {
        if (n <= 0) return 1;
        int power = 1;
        while (power < n) {
            power *= 2;
        }
        return power;
    }
    
    // Find buddy address
    int getBuddyAddress(int address, int size) {
        return address ^ size;  // XOR to find buddy
    }
    
    // Check if two blocks are buddies
    bool areBuddies(const Block& b1, const Block& b2) {
        return b1.size == b2.size && 
               getBuddyAddress(b1.address, b1.size) == b2.address;
    }
    
    // Split a block
    void splitBlock(int size) {
        // Find smallest available block >= size * 2
        for (int s = size * 2; s <= totalMemory; s *= 2) {
            if (!freeList[s].empty()) {
                // Found a block to split
                Block block = freeList[s].back();
                freeList[s].pop_back();
                
                // Split into two buddies
                int newSize = s / 2;
                Block buddy1(newSize, block.address);
                Block buddy2(newSize, block.address + newSize);
                
                freeList[newSize].push_back(buddy1);
                freeList[newSize].push_back(buddy2);
                
                cout << "  Split block at " << block.address 
                     << " (size " << s << ") into two blocks of size " << newSize << endl;
                
                // Continue splitting if needed
                if (newSize > size) {
                    splitBlock(size);
                }
                
                return;
            }
        }
    }
    
public:
    // Constructor
    BuddySystem(int totalMem, int minBlock = 64) 
        : totalMemory(totalMem), minBlockSize(minBlock), nextId(0) {
        
        // Initialize with one large free block
        Block initialBlock(totalMemory, 0);
        freeList[totalMemory].push_back(initialBlock);
        
        cout << "Buddy System initialized:" << endl;
        cout << "Total Memory: " << totalMemory << " KB" << endl;
        cout << "Minimum Block Size: " << minBlockSize << " KB" << endl;
    }
    
    // Allocate memory
    int allocate(int requestedSize) {
        cout << "\n=== Allocate " << requestedSize << " KB ===" << endl;
        
        if (requestedSize <= 0) {
            cout << "Error: Invalid size" << endl;
            return -1;
        }
        
        // Round up to next power of 2, minimum minBlockSize
        int allocSize = max(minBlockSize, nextPowerOf2(requestedSize));
        
        cout << "Rounded to: " << allocSize << " KB" << endl;
        
        // Check if block of this size is available
        if (freeList[allocSize].empty()) {
            // Try to split larger block
            cout << "No free block of size " << allocSize << ", attempting to split..." << endl;
            splitBlock(allocSize);
        }
        
        // Check again
        if (freeList[allocSize].empty()) {
            cout << "Error: Out of memory!" << endl;
            return -1;
        }
        
        // Allocate block
        Block block = freeList[allocSize].back();
        freeList[allocSize].pop_back();
        
        block.allocated = true;
        block.id = nextId++;
        
        allocatedBlocks.push_back(block);
        
        cout << "Allocated block ID " << block.id 
             << " at address " << block.address 
             << " (size " << block.size << " KB)" << endl;
        
        // Calculate fragmentation
        int internalFrag = block.size - requestedSize;
        cout << "Internal fragmentation: " << internalFrag << " KB" << endl;
        
        return block.id;
    }
    
    // Deallocate memory
    void deallocate(int blockId) {
        cout << "\n=== Deallocate Block ID " << blockId << " ===" << endl;
        
        // Find allocated block
        auto it = find_if(allocatedBlocks.begin(), allocatedBlocks.end(),
                         [blockId](const Block& b) { return b.id == blockId; });
        
        if (it == allocatedBlocks.end()) {
            cout << "Error: Block ID not found!" << endl;
            return;
        }
        
        Block block = *it;
        allocatedBlocks.erase(it);
        
        cout << "Freed block at address " << block.address 
             << " (size " << block.size << " KB)" << endl;
        
        // Try to merge with buddy
        block.allocated = false;
        block.id = -1;
        
        bool merged = true;
        while (merged && block.size < totalMemory) {
            merged = false;
            int buddyAddr = getBuddyAddress(block.address, block.size);
            
            // Look for buddy in free list
            auto& sizeList = freeList[block.size];
            auto buddyIt = find_if(sizeList.begin(), sizeList.end(),
                                  [buddyAddr](const Block& b) { 
                                      return b.address == buddyAddr && !b.allocated; 
                                  });
            
            if (buddyIt != sizeList.end()) {
                // Found buddy, merge!
                cout << "  Merging with buddy at " << buddyAddr << endl;
                
                Block buddy = *buddyIt;
                sizeList.erase(buddyIt);
                
                // Create merged block
                int newAddress = min(block.address, buddy.address);
                int newSize = block.size * 2;
                block = Block(newSize, newAddress);
                
                cout << "  Created merged block at " << newAddress 
                     << " (size " << newSize << " KB)" << endl;
                
                merged = true;
            }
        }
        
        // Add to free list
        freeList[block.size].push_back(block);
    }
    
    // Display memory status
    void displayMemoryStatus() {
        cout << "\n=== Memory Status ===" << endl;
        
        // Display allocated blocks
        cout << "\nAllocated Blocks:" << endl;
        if (allocatedBlocks.empty()) {
            cout << "  None" << endl;
        } else {
            cout << left << setw(8) << "ID" 
                 << setw(12) << "Address" 
                 << setw(12) << "Size (KB)" << endl;
            cout << string(35, '-') << endl;
            
            for (const Block& b : allocatedBlocks) {
                cout << left << setw(8) << b.id
                     << setw(12) << b.address
                     << setw(12) << b.size << endl;
            }
        }
        
        // Display free blocks
        cout << "\nFree Blocks:" << endl;
        bool hasFree = false;
        
        for (auto& pair : freeList) {
            if (!pair.second.empty()) {
                hasFree = true;
                cout << "  Size " << pair.first << " KB: ";
                for (const Block& b : pair.second) {
                    cout << b.address << " ";
                }
                cout << "(" << pair.second.size() << " blocks)" << endl;
            }
        }
        
        if (!hasFree) {
            cout << "  None" << endl;
        }
    }
    
    // Calculate fragmentation
    void displayFragmentation() {
        cout << "\n=== Fragmentation Analysis ===" << endl;
        
        // Calculate total allocated
        int totalAllocated = 0;
        for (const Block& b : allocatedBlocks) {
            totalAllocated += b.size;
        }
        
        // Calculate total free
        int totalFree = 0;
        for (auto& pair : freeList) {
            totalFree += pair.first * pair.second.size();
        }
        
        cout << "Total Memory: " << totalMemory << " KB" << endl;
        cout << "Allocated: " << totalAllocated << " KB (" 
             << (totalAllocated * 100.0 / totalMemory) << "%)" << endl;
        cout << "Free: " << totalFree << " KB (" 
             << (totalFree * 100.0 / totalMemory) << "%)" << endl;
        
        // External fragmentation
        int largestFreeBlock = 0;
        for (auto& pair : freeList) {
            if (!pair.second.empty()) {
                largestFreeBlock = max(largestFreeBlock, pair.first);
            }
        }
        
        cout << "Largest free block: " << largestFreeBlock << " KB" << endl;
        
        if (totalFree > 0) {
            cout << "External fragmentation: " 
                 << ((totalFree - largestFreeBlock) * 100.0 / totalFree) << "%" << endl;
        }
    }
};

int main() {
    cout << "=== Buddy System Memory Allocator ===" << endl;
    
    // Test case 1: Basic allocation and deallocation
    cout << "\n*** Test 1: Basic Operations ***" << endl;
    
    BuddySystem buddy(1024, 64);  // 1024 KB total, 64 KB minimum
    
    buddy.displayMemoryStatus();
    
    // Allocate some blocks
    int id1 = buddy.allocate(70);   // Will allocate 128 KB
    int id2 = buddy.allocate(35);   // Will allocate 64 KB
    int id3 = buddy.allocate(80);   // Will allocate 128 KB
    
    buddy.displayMemoryStatus();
    buddy.displayFragmentation();
    
    // Deallocate a block
    buddy.deallocate(id2);
    
    buddy.displayMemoryStatus();
    buddy.displayFragmentation();
    
    // Test case 2: Demonstrate merging
    cout << "\n\n*** Test 2: Buddy Merging ***" << endl;
    
    BuddySystem buddy2(512, 32);
    
    int b1 = buddy2.allocate(30);
    int b2 = buddy2.allocate(30);
    int b3 = buddy2.allocate(30);
    int b4 = buddy2.allocate(30);
    
    buddy2.displayMemoryStatus();
    
    cout << "\nDeallocating buddies..." << endl;
    buddy2.deallocate(b1);
    buddy2.deallocate(b2);  // Should merge with b1
    
    buddy2.displayMemoryStatus();
    
    // Test case 3: Fragmentation example
    cout << "\n\n*** Test 3: Fragmentation Analysis ***" << endl;
    
    BuddySystem buddy3(1024, 64);
    
    int x1 = buddy3.allocate(100);
    int x2 = buddy3.allocate(200);
    int x3 = buddy3.allocate(150);
    int x4 = buddy3.allocate(50);
    
    buddy3.displayMemoryStatus();
    buddy3.displayFragmentation();
    
    buddy3.deallocate(x2);
    buddy3.deallocate(x4);
    
    buddy3.displayMemoryStatus();
    buddy3.displayFragmentation();
    
    // Test case 4: Out of memory
    cout << "\n\n*** Test 4: Out of Memory ***" << endl;
    
    BuddySystem buddy4(256, 32);
    
    int m1 = buddy4.allocate(100);  // 128 KB
    int m2 = buddy4.allocate(100);  // 128 KB
    int m3 = buddy4.allocate(100);  // Should fail
    
    buddy4.displayMemoryStatus();
    
    return 0;
}
