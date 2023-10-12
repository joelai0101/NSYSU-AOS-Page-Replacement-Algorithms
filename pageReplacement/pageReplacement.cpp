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
                int victim = memoryPageFrames.front(); memoryPageFrames.pop();
                memorySet.erase(victim);

                if (memoryMap[victim].dirty == 1) { // Write back into the disk.
                    ++performance.interrupts;
                    ++performance.diskWrites;
                }

                memoryMap[victim] = {0, 0}; // Set reference and dirty bit to 0

                // Add a new page into the memory.
                memoryPageFrames.push(pageNumber);
                memorySet.insert(pageNumber);
                memoryMap[pageNumber] = {1, dirty};
            }

            ++performance.pageFaults;
            // Need to write into the disk
        } else if (memoryMap[pageNumber].dirty == 0 && dirty == 1) { 
            memoryMap[pageNumber].dirty = dirty; 
        }
        
        // printQueue(memoryPageFrames);
    }

    return performance;
}

PerformanceReport PageReplacement::SecondChance() {
    performance.reset();
    performance.algorithmName = "SecondChance";
    deque<int> memoryPageFrames;
    unordered_set<int> memorySet;
    unordered_map<int, Bits> memoryMap;

    // Execute SecondChance algorithm
    for (const auto &p : pages) {
        const int pageNumber = p[0];
        const int dirty = p[1];

        // Check if the page exists in memory with Hash Set
        if (memorySet.find(pageNumber) == memorySet.end()) { 
            if (memoryPageFrames.size() < memorySize) {
                memoryPageFrames.push_back(pageNumber);
                memorySet.insert(pageNumber);
                memoryMap[pageNumber] = {1, dirty};
            } else {
                // A memory is full and the page isn't found in the memory.
                // Choose and Remove a victim page from the memory.

                int victim = memoryPageFrames.front(); // FIFO
                while (memoryMap[victim].ref == 1) { // Second Chance
                    memoryMap[victim].ref = 0;
                    memoryPageFrames.pop_front();
                    memoryPageFrames.push_back(victim);
                }
                
                memoryPageFrames.pop_front();
                memorySet.erase(victim);

                if (memoryMap[victim].dirty == 1) { // Write back into the disk.
                    ++performance.interrupts;
                    ++performance.diskWrites;
                }

                memoryMap[victim] = {0, 0};

                memoryPageFrames.push_back(pageNumber);
                memorySet.insert(pageNumber);
                memoryMap[pageNumber] = {1, dirty};
            }

            ++performance.pageFaults;
            // Need to write into the disk
        } else if (memoryMap[pageNumber].dirty == 0 && dirty == 1) { 
            memoryMap[pageNumber].dirty = dirty; 
        }
        
        // printQueue(memoryPageFrames);
    }

    return performance;
}

PerformanceReport PageReplacement::EnhancedSecondChance() {
    performance.reset();
    performance.algorithmName = "EnhancedSecondChance";
    deque<int> memoryPageFrames;
    unordered_set<int> memorySet;
    unordered_map<int, Bits> memoryMap;

    // Execute Enhanced Second Chance algorithm
    for (const auto &p : pages) {
        const int pageNumber = p[0];
        const int dirty = p[1];

        // Check if the page exists in memory with Hash Set
        if (memorySet.find(pageNumber) == memorySet.end()) { 
            if (memoryPageFrames.size() < memorySize) {
                memoryPageFrames.push_back(pageNumber);
                memorySet.insert(pageNumber);
                memoryMap[pageNumber] = {1, dirty};
            } else {
                // A memory is full and the page isn't found in the memory.
                // Choose and Remove a victim page from the memory.

                int victim = memoryPageFrames.front(); // FIFO
                bool foundVictim = false;
                
                // Find the victim page based on the preference order
                // Perform up to four passes over the circular queue, considering pages in each class at a time.
                for (int i = 0; i < 4; ++i) {
                    for (const auto &page : memoryPageFrames) {
                        Bits bits = memoryMap[page];
                        
                        // Choose the first page encountered in the lowest nonempty class.
                        if (bits.ref == (i / 2) % 2 && bits.dirty == i % 2) {
                            victim = page;
                            foundVictim = true;
                            break;
                        } else { // Second Chance
                            memoryMap[victim].ref = 0;
                            memoryPageFrames.pop_front();
                            memoryPageFrames.push_back(victim);
                        }
                        
                    }

                    if (foundVictim) {
                        break;
                    }
                }

                memoryPageFrames.pop_front();
                memorySet.erase(victim);

                if (memoryMap[victim].dirty == 1) { // Write back into the disk.
                    ++performance.interrupts;
                    ++performance.diskWrites;
                }

                memoryMap[victim] = {0, 0};

                memoryPageFrames.push_back(pageNumber);
                memorySet.insert(pageNumber);
                memoryMap[pageNumber] = {1, dirty};
                
            }

            ++performance.pageFaults;
            // Need to write into the disk
        } else if (memoryMap[pageNumber].dirty == 0 && dirty == 1) { 
            memoryMap[pageNumber].dirty = dirty; 
        }
        
        // printQueue(memoryPageFrames);
    }

    return performance;
}

// Optimal algorithm
PerformanceReport PageReplacement::Optimal() {
    performance.reset();
    performance.algorithmName = "Optimal";
    vector<int> memoryPageFrames;
    unordered_map<int, Bits> memoryMap;
    
    // 1. Execute optimal algorithm.
    for (int i = 0; i < pages.size(); ++i) {
        const int pageNumber = pages[i][0];
        const int dirty = pages[i][1];
        
        if (find(memoryPageFrames.begin(), memoryPageFrames.end(), pageNumber) == memoryPageFrames.end()) {
            if (memoryPageFrames.size() < memorySize) {
                memoryPageFrames.push_back(pageNumber);

                memoryMap[pageNumber] = {1, dirty};
            } else {

                const int j = predict(i + 1, memoryPageFrames);
                int victim = memoryPageFrames[j];
                memoryPageFrames[j] = pageNumber;
                memoryMap[pageNumber] = {1, dirty};
                if (memoryMap[victim].dirty == 1) {
                    ++performance.diskWrites;
                    ++performance.interrupts;
                }
                memoryMap[victim] = {0, 0};
            }
            ++performance.pageFaults;
        } else if (memoryMap[pageNumber].dirty == 0 && dirty == 1) { 
            memoryMap[pageNumber].dirty = dirty; 
        }
        // printVec(memory);
    }
    return performance;
}

// Find a victim for optimal
int PageReplacement::predict(const int index, const vector<int> &memory){
    int pre = -1, farthest = index;
    for (int i = 0; i < memory.size(); ++i) {
        int j;
        // Store the index (j) of pages which are going to be used recently in future
        for (j = index; j < pages.size(); ++j) {
            if (memory[i] == pages[j][0]) {
                if (j > farthest) {
                    farthest = j;
                    pre = i;
                }
                break;
            }
        }
        // If a page is never used in future, return it.
        if (j == pages.size()) {return i;}
    }
    // If all of the frames were not in future, return any of them, we return 0.
    // Otherwise we return pre.
    return pre == -1 ? 0 : pre;
}

// Additional-reference-bits (ARB) algorithm
// An additional reference bit is used to determine the victim, 
// usually using 8 bits and updating the ARB for all pages in memory
PerformanceReport PageReplacement::ARB(const int interval) {
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
                int victim = memoryPageFrames[j];
                
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
            if (memoryMap[pageNumber].dirty == 0 && dirty == 1) { 
                memoryMap[pageNumber].dirty = dirty; 
            }
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