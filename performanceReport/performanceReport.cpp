#include "performanceReport.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;
namespace fs = std::__fs::filesystem;

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

void PerformanceReport::writeCsvReport(const string referenceStringName, int memorySize) {
    fstream csvFile;
    // cout << memorySize << endl; // check size
    string dataDir = "../data";
    fs::create_directory(dataDir);

    csvFile.open("../data/" + algorithmName + ".csv", ios::in | ios::app | ios::out);
    if (csvFile.is_open()) {
        csvFile.seekg(0, ios::end); // 檔案指標移到末尾
        streampos csvFileSize = csvFile.tellg(); // 取得指標位置，即檔案大小
        if (csvFileSize == 0) {
            csvFile << "algorithmName" << ","<< referenceStringName << "," << "memorySize" << "," << "pageFaults" << "," << "interrupts" << "," << "diskWrites" << endl;
            csvFile << algorithmName << ","<< referenceStringName << "," << memorySize << "," << pageFaults << "," << interrupts << "," << diskWrites << endl;
        } else {
            csvFile << algorithmName << "," << referenceStringName << "," << memorySize << "," << pageFaults << "," << interrupts << "," << diskWrites << endl;
        }

        csvFile.close();
    } else { 
        cerr << "Failed to open file. \n";
    }
}