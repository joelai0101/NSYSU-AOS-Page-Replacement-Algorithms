#include "../performanceReport/performanceReport.hpp"
#include "pageReplacement.hpp"
#include <fstream>
#include <iostream>
#include <queue>

using namespace std;

void printQueue(queue<int> q){
    if (q.size() != 0) {
        do {
            cout << q.front() << " "; q.pop();
        } while(q.size() > 0);
    }
    cout << endl;
}

void printVector(const vector<int> &vec) {
    for (const auto v : vec) { cout << v << " "; }
    cout << endl;
}

PageReplacement::PageReplacement(const int p_memorySize, const string p_fileName) 
    : memorySize(p_memorySize) {
        setFileName(p_fileName);
    }

void PageReplacement::setFileName(const string p_fileName) {
    if (fileName != p_fileName) {
        fileName = p_fileName;
        
        // Open a file and check if it is opened.
        ifstream file(fileName);
        if (!file) { cerr << "File don't be opened." << endl; }
        
        // Store all content of the file.
        int pageNumber, dirty;
        pages.clear();
        // Column 0 is a page number and column 1 is a dirty bit.
        while (file >> pageNumber >> dirty) { pages.push_back({pageNumber, dirty}); }
        file.close();
    }
}

PerformanceReport PageReplacement::FIFO() { 
    // init
    performance.reset();
    performance.algorithmName = "FIFO";
    queue<int> memoryPageFrames; // Simulate page frames in memory with queue
    unordered_set<int> memorySet; // Track whether page frames in memory are used with unordered_set
    unordered_map<int, Bits> memoryMap; // Track the reference bit and dirty bit of each page frame with unordered_map
    // <page number, Bits>

    // Execute FIFO algorithm
    for (const auto &p : pages) {
        // Column 0 is a page number and column 1 is a dirty bit.
        const int pageNumber = p[0];
        const int dirty = p[1];

        // Check if the page exists in memory with Hash Set
        if (memorySet.find(pageNumber) == memorySet.end()) { 
            if (memoryPageFrames.size() < memorySize) {
                // A memory isn't full and the page isn't found in the memory.
                // Add a new page into the memory.
                memoryPageFrames.push(pageNumber);
                memorySet.insert(pageNumber);
                memoryMap[pageNumber] = {1, dirty};
            } else {
                // A memory is full and the page isn't found in the memory.
                // Choose and Remove a victim page from the memory.
                // that is, the page that entered the queue earliest.
                const int victim = memoryPageFrames.front(); memoryPageFrames.pop();
                memorySet.erase(victim);

                if (memoryMap[victim].dirty == 1) { // Write back into the disk.
                    ++performance.interrupts;
                    ++performance.diskWrites;
                }

                memoryMap[victim] = {0, 0};
                memoryMap[victim].ref = 0; // set reference bit to 0

                // Add a new page into the memory.
                memoryPageFrames.push(pageNumber);
                memorySet.insert(pageNumber);
                memoryMap[pageNumber] = {1, dirty};
            }

            ++performance.pageFaults;
            // Need to write into the disk
        } else if (memoryMap[pageNumber].dirty == 0 && dirty == 1) { memoryMap[pageNumber].dirty = dirty; }
        
        // printQueue(memoryPageFrames);
    }

    return performance;
}

// Additional-reference-bits (ARB) algorithm
// An additional reference bit is used to determine the victim, 
// usually using 8 bits and updating the ARB for all pages in memory
PerformanceReport PageReplacement::ARB(const int interval) {
    // init
    performance.reset();
    performance.algorithmName = "ARB";
    int count = 0;
    vector<int> memoryPageFrames; // A vector to store page frames in memory
    unordered_map<int, Bits> memoryMap;
    unordered_set<int> memoryHits; // Track hit page frames in memory
    
    // Excute Additional-reference-bits (ARB)
    for (const auto &p : pages) {
        int isInterrupt = 0; // init 
        // Column 0 is a page number and column 1 is a dirty bit.
        const int pageNumber = p[0];
        const int dirty = p[1];

        // Check if the page exists in memory with vector
        if (find(memoryPageFrames.begin(), memoryPageFrames.end(), pageNumber) == memoryPageFrames.end()) {
            if (memoryPageFrames.size() < memorySize) {
                // A memory isn't full and the page isn't found in the memory.
                // Add a new page into the memory.
                memoryPageFrames.push_back(pageNumber);
                // The most significant bit (MSB) of a page that has been referenced recently will be '1'
                memoryMap[pageNumber] = {128, dirty}; // 128 = 2^8 = 1000 0000(8-bit number)
            } else {
                // A memory is full and the page isn't found in the memory.
                // We should choose and remove a victim page from the memory.
                // To get and remove a victim with the least significant bit (LSB) (that is, the least referenced page),
                // We need to know the position of the minimal reference bit
                const int j = findMinRefBit(memoryPageFrames, memoryMap);
                const int victim = memoryPageFrames[j];
                
                // replace the victim with new page
                memoryPageFrames[j] = pageNumber;
                memoryMap[pageNumber] = {128, dirty};
                if (memoryMap[victim].dirty == 1) {
                    ++performance.diskWrites;
                    ++performance.interrupts;
                    isInterrupt = 1;
                }
                memoryMap[victim] = {0, 0};
            }

            ++performance.pageFaults;

        } else {
            memoryHits.insert(pageNumber);
            if (memoryMap[pageNumber].dirty == 0 && dirty == 1) { memoryMap[pageNumber].dirty = dirty; }
        }
        // printVector(memoryPageFrames);
        
        // Update the reference bit of all pages in the memory.
        if (++count == interval) {
            count = 0;
            updateARB(memoryPageFrames ,memoryMap, memoryHits);
            if (!isInterrupt) { ++performance.interrupts; }
        }
    }

    return performance;
}

// Find a victim for ARB
int PageReplacement::findMinRefBit(const vector<int> &memoryPageFrames, unordered_map<int, Bits> &memoryMap) {
    int min = 256; // 8-bit information
    int minIndex = 0;
    // iterate through all page frames in the memory
    for (int i = 0; i < memoryPageFrames.size(); ++i) {
        if (memoryMap[memoryPageFrames[i]].ref < min) { 
        // check if a reference bit < current min value
            minIndex = i;
            min = memoryMap[memoryPageFrames[i]].ref;
        }
    }
    return minIndex;
}

void PageReplacement::updateARB(const vector<int> &memoryPageFrames, unordered_map<int, Bits> &memoryMap, unordered_set<int> &memoryHits) {
    // Shift right the reference bit of all pages in the memory by 1 bit.
    for (const auto fm : memoryPageFrames) { memoryMap[fm].ref >>= 1; }
    
    // If pages in the memory are referenced, their reference bit ^ 1000 0000(2).
    for (const auto &h : memoryHits) { memoryMap[h].ref &= 128; }
    memoryHits.clear();
}