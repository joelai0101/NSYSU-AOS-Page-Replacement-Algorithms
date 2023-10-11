#ifndef __performanceReport__
#define __performanceReport__

#include <string>

using namespace std;

class PerformanceReport {
public:
    void reset() {
        pageFaults = 0;
        interrupts = 0;
        diskWrites = 0;
        algorithmName = "";
    }
    
    void printReport(const int n = 1);
    
    int pageFaults, interrupts, diskWrites;
    string algorithmName;
};

#endif // __performanceReport__