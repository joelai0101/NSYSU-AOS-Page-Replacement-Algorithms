#ifndef __pageReplacement__
#define __pageReplacement__

#include "../performanceReport/performanceReport.hpp"
#include <string>
#include <unordered_set>
#include <unordered_map>

using namespace std;

typedef struct Bits { // You can use both reference and dirty bits
    int ref; // reference bit
    int dirty; // dirty bit
} Bits;

class PageReplacement {
public:
    PageReplacement(const int p_memorySize, const string p_fileName);
    ~PageReplacement() {}

    void setMemorySize(const int p_memorySize) { memorySize = p_memorySize; }
    void setFileName(const string p_fileName);
    int getFileSize() { return pages.size(); }

    // Algorithms
    PerformanceReport FIFO();
    PerformanceReport ARB(const int interval = 1);
    // PerformanceReport EnhancedSecondChance();

private:
    PerformanceReport performance;
    int memorySize;
    string fileName;
    vector<vector<int>> pages;
    
    int findMinRefBit(const vector<int> &memory, unordered_map<int, Bits> &memortBits); // Find a victim for ARB
    void updateARB(const vector<int> &memory, unordered_map<int, Bits> &memortBits, unordered_set<int> &memoryHit); // For ARB
};

#endif // __pageReplacement__