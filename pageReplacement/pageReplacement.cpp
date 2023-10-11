#include "../performanceReport/performanceReport.hpp"
#include "pageReplacement.hpp"
#include <fstream>
#include <iostream>
#include <queue>

using namespace std;

PageReplacement::PageReplacement(const int p_memorySize, const string p_fileName) 
    : memorySize(p_memorySize) {
        setFileName(p_fileName);
    }

void PageReplacement::setFileName(const string p_fileName) {
    if (fileName != p_fileName) {
        fileName = p_fileName;
        
        // 1. Open a file and check if it is opened.
        ifstream file(fileName);
        if (!file) { cerr << "File don't be opened." << endl; }
        
        // 2. Pages store all content of athe file.
        int ref, dirtyBit;
        pages.clear();
        // Column 0 is a ref and column 1 is a dirty bit.
        while (file >> ref >> dirtyBit) { pages.push_back({ref, dirtyBit}); }
        file.close();
    }
}

PerformanceReport PageReplacement::FIFO() { 
    performance.reset();
    performance.algorithmName = "FIFO";
    queue<int> memory;
    unordered_set<int> memorySet;
    unordered_map<int, Bits> memoryBits;

    // 1. Execute FIFO algorithm
    for (const auto &p : pages) {
        // Column 0 is a ref and column 1 is a dirty bit.
        const int ref = p[0];
        const int dirtyBit = p[1];
        if (memorySet.find(ref) == memorySet.end()) {
            if (memory.size() < memorySize) {
                // 1.1 A memory isn't full and ref isn't found in the memory.
                // 1.1.1 Add a new ref into the memory.
                memory.push(ref);
                memorySet.insert(ref);
                memoryBits[ref] = {1, dirtyBit};
            } else {
                // 1.2 A memory is full and ref isn't found in the memory.
                // 1.2.1 Choose and Remove a victim from the memory.
                const int victim = memory.front();
                memory.pop();
                memorySet.erase(victim);
                if (memoryBits[victim].dirty == 1) { // Write back into the disk.
                    ++performance.interrupts;
                    ++performance.diskWrites;
                }
                memoryBits[victim] = {0, 0};
                // 1.2.2 Add a new ref into the memory.
                memoryBits[victim].ref = 0;
                memory.push(ref);
                memorySet.insert(ref);
                memoryBits[ref] = {1, dirtyBit};
            }
            ++performance.pageFaults;
        } else if (memoryBits[ref].dirty == 0 && dirtyBit == 1) { // diskWrites
            memoryBits[ref].dirty = dirtyBit;
        }
        // printQueue(memory);
    }

    return performance;
}

// Additional-reference-bits (ARB) algorithm
PerformanceReport PageReplacement::ARB(const int interval) {
    performance.reset();
    performance.algorithmName = "ARB";
    int count = 0;
    vector<int> memory;
    unordered_map<int, Bits> memoryBits;
    unordered_set<int> memoryHits;
    
    // 1. Excute Additional-reference-bits (ARB)
    for (const auto &p : pages) {
        int isInterrupt = 0;
        // Column 0 is a ref and column 1 is a dirty bit.
        const int ref = p[0];
        const int dirtyBit = p[1];
        if (find(memory.begin(), memory.end(), ref) == memory.end()) {
            if (memory.size() < memorySize) {
                // 1.1 A memory isn't full and ref isn't found in the memory.
                // Add a new ref into the memory.
                memory.push_back(ref);
                memoryBits[ref] = {128, dirtyBit};
            } else {
                // 1.2 A memory is full and ref isn't found in the memory.
                // Choose and Remove a victim from the memory.
                const int j = findMinRef(memory, memoryBits);
                const int victim = memory[j];
                memory[j] = ref;
                memoryBits[ref] = {128, dirtyBit};
                if (memoryBits[victim].dirty == 1) {
                    ++performance.diskWrites;
                    ++performance.interrupts;
                    isInterrupt = 1;
                }
                memoryBits[victim] = {0, 0};
            }
            ++performance.pageFaults;
        } else {
            memoryHits.insert(ref);
            if (memoryBits[ref].dirty == 0 && dirtyBit == 1) { memoryBits[ref].dirty = dirtyBit; }
        }
        // printVec(memory);
        
        // 1.2 Update the refBit of all pages in the memory.
        if (++count == interval) {
            count = 0;
            updateARB(memory ,memoryBits, memoryHits);
            if (!isInterrupt) { ++performance.interrupts; }
        }
    }

    return performance;
}

// Find a victim for ARB
int PageReplacement::findMinRef(const vector<int> &memory, unordered_map<int, Bits> &memoryBits) {
    int min = 256; // 8-bit information
    int minIndex = 0;
    for (int i = 0; i < memory.size(); ++i) {
        if (memoryBits[memory[i]].ref < min) {
            minIndex = i;
            min = memoryBits[memory[i]].ref;
        }
    }
    return minIndex;
}

void PageReplacement::updateARB(const vector<int> &memory, unordered_map<int, Bits> &memoryBits, unordered_set<int> &memoryHits) {
    // 1. Shfit right the refBit of all pages in the memory by 1 bit.
    for (const auto m : memory) { memoryBits[m].ref >>= 1; }
    
    // 2. If pages in the memory are referenced, their refBit ^ 1000 0000(2).
    for (const auto &h : memoryHits) { memoryBits[h].ref &= 128; }
    memoryHits.clear();
}