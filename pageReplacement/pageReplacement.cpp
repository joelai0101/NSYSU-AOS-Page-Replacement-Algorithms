#include "../performanceReport/performanceReport.hpp"
#include "pageReplacement.hpp"
#include <fstream>
#include <iostream>
#include <queue>
#include <list>

using namespace std;

void printQueue(queue<int> q) {
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
    unordered_map<int, Bits> bitMap; // Track the reference bit and dirty bit of each page frame with unordered_map
    // <page number, Bits>

    // Execute FIFO algorithm
    for (const auto &p : pages) {
        const int pageNumber = p[0];
        const int dirty = p[1];

        // Check if the page exists in memory with Hash Set
        if (memorySet.find(pageNumber) == memorySet.end()) { // If page doesn't exist in memory
            ++performance.pageFaults;  // Page fault occurs when the page is not found in memory.
            ++performance.interrupts;  // An interrupt is generated when a page fault occurs.

            if (memoryPageFrames.size() < memorySize) {
                // A memory isn't full and the page isn't found in the memory.
                // Add a new page into the memory.
                memoryPageFrames.push(pageNumber);
                memorySet.insert(pageNumber);
                bitMap[pageNumber] = {0, dirty};  // Set the dirty bit according to the input.
            } else {
                // A memory is full and the page isn't found in the memory.
                // Choose and Remove a victim page from the memory.
                // that is, the page that entered the queue earliest.
                int victim = memoryPageFrames.front(); memoryPageFrames.pop();
                memorySet.erase(victim);

                if (bitMap[victim].dirty == 1) { // Write back into the disk.
                    ++performance.diskWrites;
                    ++performance.interrupts;
                    bitMap[victim].dirty = 0;
                }

                // Add a new page into the memory.
                memoryPageFrames.push(pageNumber);
                memorySet.insert(pageNumber);
                bitMap[pageNumber] = {0, dirty};  // Set the dirty bit according to the input.
            }
        } else {
            // The page is found in memory. Set its reference bit to 1.
            bitMap[pageNumber].ref = 1;
            if (bitMap[pageNumber].dirty == 0 && dirty == 1) { 
                bitMap[pageNumber].dirty = dirty; 
            }
        }
        
        // printQueue(memoryPageFrames);
    }

    return performance;
}

PerformanceReport PageReplacement::SecondChance() {
    performance.reset();
    performance.algorithmName = "Second Chance";
    deque<int> memoryPageFrames;
    unordered_set<int> memorySet;
    unordered_map<int, Bits> bitMap;

    // Execute SecondChance algorithm
    for (const auto &p : pages) {
        const int pageNumber = p[0];
        const int dirty = p[1];

        // Check if the page exists in memory with Hash Set
        if (memorySet.find(pageNumber) == memorySet.end()) { 
            ++performance.pageFaults;  // Page fault occurs when the page is not found in memory.
            ++performance.interrupts;  // An interrupt is generated when a page fault occurs.

            if (memoryPageFrames.size() < memorySize) {
                memoryPageFrames.push_back(pageNumber);
                memorySet.insert(pageNumber);
                // 將其參考位元設為 1 是因為該頁面剛被加載到記憶體中，我們假設它將被立即使用。
                bitMap[pageNumber] = {1, dirty}; 
            } else {
                // A memory is full and the page isn't found in the memory.
                // Choose and Remove a victim page from the memory.

                while (true) {
                    int victim = memoryPageFrames.front(); // FIFO

                    if (bitMap[victim].ref == 0) {  // If the reference bit is 0, remove it.
                        if (bitMap[victim].dirty == 1) { // Write back into the disk.
                            ++performance.diskWrites;
                            ++performance.interrupts;
                            bitMap[victim].dirty = 0;
                        }

                        memoryPageFrames.pop_front();
                        memorySet.erase(victim);
                        break;
                    } else {  // If the reference bit is 1, give it a second chance and move it to the back of the queue.
                        bitMap[victim].ref = 0;
                        memoryPageFrames.pop_front();
                        memoryPageFrames.push_back(victim);
                    }
                }

                // Add a new page into the memory.
                memoryPageFrames.push_back(pageNumber);
                memorySet.insert(pageNumber);
                bitMap[pageNumber] = {1, dirty}; 
            }
        } else {
            // The page is found in memory. Set its reference bit to 1.
            bitMap[pageNumber].ref = 1;
            if (bitMap[pageNumber].dirty == 0 && dirty == 1) { 
                bitMap[pageNumber].dirty = dirty; 
            }
        }
        
        // printQueue(memoryPageFrames);
    }

    return performance;
}

PerformanceReport PageReplacement::EnhancedSecondChance() {
    performance.reset();
    performance.algorithmName = "ESC";
    deque<int> memoryPageFrames;
    unordered_set<int> memorySet;
    unordered_map<int, Bits> bitMap;
    int counter = 0;

    // Execute Enhanced Second Chance algorithm
    for (const auto &p : pages) {
        const int pageNumber = p[0];
        const int dirty = p[1];

        // Check if the page exists in memory with Hash Set
        if (memorySet.find(pageNumber) == memorySet.end()) { 
            ++performance.pageFaults;  // Page fault occurs when the page is not found in memory.
            ++performance.interrupts;  // An interrupt is generated when a page fault occurs.

            if (memoryPageFrames.size() < memorySize) {
                memoryPageFrames.push_back(pageNumber);
                memorySet.insert(pageNumber);
                // 將其參考位元設為 0 可以提高其被替換的可能，從而讓其他已在記憶體中並可能仍在使用的頁面有更多的機會保持在記憶體中。
                bitMap[pageNumber] = {0, dirty};
            } else {
                // A memory is full and the page isn't found in the memory.
                // Choose and Remove a victim page from the memory.

                int victim = memoryPageFrames.front(); // FIFO
                bool foundVictim = false;
                
                // Find the victim page based on the preference order
                // Perform up to four passes over the circular queue, considering pages in each class at a time.
                while (!foundVictim && counter < 4) {
                    for (auto &it : memoryPageFrames) {
                        if (counter == 0 && bitMap[it].ref == 0 && bitMap[it].dirty == 0) {
                            victim = it;
                            foundVictim = true;
                            break;
                        } else if (counter == 1 && bitMap[it].ref == 0 && bitMap[it].dirty == 1) {
                            victim = it;
                            foundVictim = true;
                            break;
                        } else if (counter == 2 && bitMap[it].ref == 1 && bitMap[it].dirty == 0) {
                            bitMap[it].ref = 0;
                            memoryPageFrames.pop_front();
                            memoryPageFrames.push_back(it);
                        } else if (counter == 3 && bitMap[it].ref == 1 && bitMap[it].dirty == 1) {
                            bitMap[it].ref = 0;
                            memoryPageFrames.pop_front();
                            memoryPageFrames.push_back(it);
                        }
                        
                    }
                    counter++;
                }
                
                if (bitMap[victim].dirty == 1) { // Write back into the disk.
                    ++performance.diskWrites;
                    ++performance.interrupts;
                    bitMap[victim].dirty = 0;
                }

                memoryPageFrames.pop_front();
                memorySet.erase(victim);

                // Add a new page into the memory.
                memoryPageFrames.push_back(pageNumber);
                memorySet.insert(pageNumber);
                bitMap[pageNumber] = {0, dirty};
                
            }
        } else {
            // The page is found in memory. Set its reference bit to 1.
            bitMap[pageNumber].ref = 1;
            if (bitMap[pageNumber].dirty == 0 && dirty == 1) { 
                bitMap[pageNumber].dirty = dirty; 
            }
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
    unordered_map<int, Bits> bitMap;
    
    // 1. Execute optimal algorithm.
    for (int i = 0; i < pages.size(); ++i) {
        const int pageNumber = pages[i][0];
        const int dirty = pages[i][1];
        
        if (find(memoryPageFrames.begin(), memoryPageFrames.end(), pageNumber) == memoryPageFrames.end()) {
            ++performance.pageFaults;  // Page fault occurs when the page is not found in memory.
            ++performance.interrupts;  // An interrupt is generated when a page fault occurs.
            
            if (memoryPageFrames.size() < memorySize) {
                memoryPageFrames.push_back(pageNumber);

                bitMap[pageNumber] = {0, dirty};
            } else {

                const int j = OptimalPredict(i + 1, memoryPageFrames);
                int victim = memoryPageFrames[j];
                
                if (bitMap[victim].dirty == 1) {
                    ++performance.diskWrites;
                    ++performance.interrupts;
                    bitMap[victim].dirty = 0;
                }

                memoryPageFrames[j] = pageNumber;
                bitMap[pageNumber] = {0, dirty};
            }
        } else {
            // The page is found in memory. Set its reference bit to 1.
            bitMap[pageNumber].ref = 1;
            if (bitMap[pageNumber].dirty == 0 && dirty == 1) { 
                bitMap[pageNumber].dirty = dirty; 
            }
        }
        // printVec(memory);
    }
    return performance;
}

// Find a victim for optimal
int PageReplacement::OptimalPredict(const int index, const vector<int> &memory) {
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
        if (j == pages.size()) { return i; }
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
    unordered_map<int, Bits> bitMap;
    unordered_set<int> memoryHits; // Track hit page frames in memory
    
    // Excute Additional-reference-bits (ARB)
    for (const auto &p : pages) {
        int isInterrupt = 0; // init 
        // Column 0 is a page number and column 1 is a dirty bit.
        const int pageNumber = p[0];
        const int dirty = p[1];

        // Check if the page exists in memory with vector
        if (find(memoryPageFrames.begin(), memoryPageFrames.end(), pageNumber) == memoryPageFrames.end()) {
            ++performance.pageFaults;  // Page fault occurs when the page is not found in memory.
            ++performance.interrupts;  // An interrupt is generated when a page fault occurs.
            
            if (memoryPageFrames.size() < memorySize) {
                // A memory isn't full and the page isn't found in the memory.
                // Add a new page into the memory.
                memoryPageFrames.push_back(pageNumber);
                // The most significant bit (MSB) of a page that has been referenced recently will be '1'
                bitMap[pageNumber] = {(1 << 7), dirty}; // 128 = 2^7 = 1000 0000(8-bit number)
            } else {
                // A memory is full and the page isn't found in the memory.
                // We should choose and remove a victim page from the memory.
                // To get and remove a victim with the least significant bit (LSB) (that is, the least referenced page),
                // We need to know the position of the minimal reference bit
                const int j = FindMinRefBit(memoryPageFrames, bitMap);
                int victim = memoryPageFrames[j];
                
                if (bitMap[victim].dirty == 1) {
                    ++performance.diskWrites;
                    ++performance.interrupts;
                    bitMap[victim].dirty = 0;
                    isInterrupt = 1;
                }

                // replace the victim with new page
                memoryPageFrames[j] = pageNumber;
                bitMap[pageNumber] = {(1 << 7), dirty};
            }
        } else {
            // The page is found in memory. Set its reference bit to 1.
            memoryHits.insert(pageNumber);
            bitMap[pageNumber].ref = bitMap[pageNumber].ref | (1 << 7);
            if (bitMap[pageNumber].dirty == 0 && dirty == 1) { 
                bitMap[pageNumber].dirty = dirty; 
            }
        }
        // printVector(memoryPageFrames);
        
        // Update the reference bit of all pages in the memory.
        if (++count == interval) {
            count = 0;
            UpdateARB(memoryPageFrames, bitMap, memoryHits);
            // if (!isInterrupt) { ++performance.interrupts; }
        }
    }

    return performance;
}

// Find a victim for ARB
int PageReplacement::FindMinRefBit(const vector<int> &memoryPageFrames, unordered_map<int, Bits> &bitMap) {
    int min = 256; // 8-bit information
    int minIndex = 0;
    // iterate through all page frames in the memory
    for (int i = 0; i < memoryPageFrames.size(); ++i) {
        if (bitMap[memoryPageFrames[i]].ref < min) { 
        // check if a reference bit < current min value
            minIndex = i;
            min = bitMap[memoryPageFrames[i]].ref;
        }
    }
    return minIndex;
}

void PageReplacement::UpdateARB(const vector<int> &memoryPageFrames, unordered_map<int, Bits> &bitMap, unordered_set<int> &memoryHits) {
    // Shift right the reference bit of all pages in the memory by 1 bit.
    for (const auto fm : memoryPageFrames) { bitMap[fm].ref >>= 1; }
    
    // If pages in the memory are referenced, their reference bit ^ 1000 0000(2).
    for (const auto &h : memoryHits) { bitMap[h].ref &= (1 << 7); }
    memoryHits.clear();
}

PerformanceReport PageReplacement::LRU() {
    // init
    performance.reset();
    performance.algorithmName = "LRU";
    list<int> memoryPageFrames; // Simulate page frames in memory with doubly linked list
    unordered_map<int, list<int>::iterator> posMap; // Track the position of each page frame in the list with unordered_map
    // <page number, iterator>
    unordered_map<int, Bits> bitMap;

    // Execute LRU algorithm
    for (const auto &p : pages) {
        const int pageNumber = p[0];
        const int dirty = p[1];

        // Check if the page exists in memory with Hash Map
        if (posMap.find(pageNumber) == posMap.end()) { // If page doesn't exist in memory
            ++performance.pageFaults;  // Page fault occurs when the page is not found in memory.
            ++performance.interrupts;  // An interrupt is generated when a page fault occurs.

            if (memoryPageFrames.size() < memorySize) {
                // A memory isn't full and the page isn't found in the memory.
                // Add a new page into the front of the list.
                memoryPageFrames.push_front(pageNumber);
                posMap[pageNumber] = memoryPageFrames.begin(); // Store the iterator of the new page
                bitMap[pageNumber] = {0, dirty};
            } else {
                // A memory is full and the page isn't found in the memory.
                // Choose and Remove a victim page from the back of the list.
                int victim = memoryPageFrames.back(); memoryPageFrames.pop_back();
                posMap.erase(victim);

                if (bitMap[victim].dirty == 1) { // Write back into the disk.
                    ++performance.diskWrites;
                    ++performance.interrupts;
                    bitMap[victim].dirty = 0;
                }

                // Add a new page into the front of the list.
                memoryPageFrames.push_front(pageNumber);
                posMap[pageNumber] = memoryPageFrames.begin(); // Store the iterator of the new page
                bitMap[pageNumber] = {0, dirty};
            }
        } else {
            // The page is found in memory. Move it to the front of the list.
            auto it = posMap[pageNumber]; // Get the iterator of the existing page
            memoryPageFrames.erase(it); // Remove it from its current position
            memoryPageFrames.push_front(pageNumber); // Insert it to the front of the list
            posMap[pageNumber] = memoryPageFrames.begin(); // Update the iterator of the existing page
            // Set its reference bit to 1.
            bitMap[pageNumber].ref = 1;
            if (bitMap[pageNumber].dirty == 0 && dirty == 1) { 
                bitMap[pageNumber].dirty = dirty; 
            }
        }
        
    }

    return performance;
}

PerformanceReport PageReplacement::LRU_LFU() {
    // init
    performance.reset();
    performance.algorithmName = "LRU-LFU";
    list<int> memoryPageFrames; // Simulate page frames in memory with list
    unordered_map<int, pair<list<int>::iterator, int>> memoryMap; // Track the position and frequency of each page frame in the list with unordered_map
    // <page number, <iterator, frequency>>
    unordered_map<int, Bits> bitsMap; // Track the reference bit and dirty bit of each page frame with unordered_map

    // Execute LRU-LFU algorithm
    for (const auto &p : pages) {
        const int pageNumber = p[0];
        const int dirty = p[1];

        // Check if the page exists in memory with Hash Map
        if (memoryMap.find(pageNumber) == memoryMap.end()) { // If page doesn't exist in memory
            ++performance.pageFaults;  // Page fault occurs when the page is not found in memory.
            ++performance.interrupts;  // An interrupt is generated when a page fault occurs.

            if (memoryPageFrames.size() < memorySize) {
                // A memory isn't full and the page isn't found in the memory.
                // Add a new page into the front of the list with frequency 1.
                memoryPageFrames.push_front(pageNumber);
                auto it = memoryPageFrames.begin(); // Store the iterator of the new page
                int freq = 1; // Set the frequency of the new page to 1
                memoryMap[pageNumber] = make_pair(it, freq); 
                bitsMap[pageNumber] = {0, dirty}; // Set the reference bit and dirty bit according to the input
            } else {
                // A memory is full and the page isn't found in the memory.
                // Choose and Remove a victim page from the list based on LRU-LFU policy.
                int victim = -1; // The page number of the victim
                int minFreq = INT_MAX; // The minimum frequency among the pages in memory
                int minIndex = -1; // The index of the page with minimum frequency in the list
                int index = 0; // The current index in the list
                for (const auto &page : memoryPageFrames) { // Traverse the list from front to back
                    auto it = memoryMap[page].first; // Get the iterator of the current page
                    int freq = memoryMap[page].second; // Get the frequency of the current page
                    if (freq < minFreq) { // Update the minimum frequency and the corresponding index and page number
                        minFreq = freq;
                        minIndex = index;
                        victim = page;
                    } else if (freq == minFreq && index > minIndex) { // If there are multiple pages with the same minimum frequency, choose the LRU one
                        minIndex = index;
                        victim = page;
                    }
                    ++index; // Increase the current index by 1
                }
                auto minIt = std::next(memoryPageFrames.begin(), minIndex); // Get the iterator of the victim page by adding its index to the begin iterator
                memoryPageFrames.erase(minIt); // Remove the victim page from the list
                memoryMap.erase(victim); // Remove the victim page from the map

                if (bitsMap[victim].dirty == 1) { // Write back into the disk.
                    ++performance.diskWrites;
                    ++performance.interrupts;
                    bitsMap[victim].dirty = 0;
                }

                // Add a new page into the front of the list with frequency 1.
                memoryPageFrames.push_front(pageNumber);
                auto it = memoryPageFrames.begin(); // Store the iterator of the new page
                int freq = 1; // Set the frequency of the new page to 1
                memoryMap[pageNumber] = make_pair(it, freq); 
                bitsMap[pageNumber] = {0, dirty};  // Set the reference bit and dirty bit according to the input
            }
        } else {
            // The page is found in memory. Move it to the front of the list and increase its frequency by 1.
            auto it = memoryMap[pageNumber].first; // Get the iterator of the existing page
            int freq = memoryMap[pageNumber].second; // Get the frequency of the existing page
            memoryPageFrames.erase(it); // Remove it from its current position
            memoryPageFrames.push_front(pageNumber); // Insert it to the front of the list
            ++freq; // Increase its frequency by 1
            memoryMap[pageNumber] = make_pair(memoryPageFrames.begin(), freq); // Update the iterator and frequency of the existing page
            // Set its reference bit to 1.
            bitsMap[pageNumber].ref = 1;
            if (bitsMap[pageNumber].dirty == 0 && dirty == 1) { 
                bitsMap[pageNumber].dirty = dirty; 
            }
        }
    }

    return performance;
}

