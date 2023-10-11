#include "performanceReport.hpp"
#include <iostream>

using namespace std;

void PerformanceReport::printReport(const int n) {
    switch (n) {
        case 1:
            cout << "Algorithm: " << algorithmName << endl;
            cout << "Page faults: " << pageFaults << endl;
            cout << "Interrupts: " << interrupts << endl;
            cout << "Disk writes: " << diskWrites << endl;
            cout << endl;
            break;
            
        case 2:
            cout << pageFaults << " " << interrupts << " " << diskWrites << endl;
            break;
    }
}