#include "referenceString/referenceString.hpp"
#include "performanceReport/performanceReport.hpp"
#include "pageReplacement/pageReplacement.hpp"
#include <iostream>

using namespace std;

int main(int argc, const char * argv[]) {
    // init parameters of three different test reference strings.
    const int dataSize = 200000; // Number of memory references: At least 200,000 times
    const int referenceSize = 1000; // page Reference string: 1~1,000
    const double dirtyRate = 0.5;
    const double subsetRateA = 1.0 / 30.0;
    const double subsetRateB = 1.0 / 20.0;
    int referenceRange = 20;
    double setSize = 20;
    double lambda = 1.0 / referenceSize; // 期望平均值與參考字串的大小 (Ref. size) 相關

    vector<int> memorySize = {20, 40, 60, 80, 100}; // Number of frames in the physical memory
    vector<string> fileName = {"uniform_reference_string.txt", "locality_reference_string.txt", "exponential_reference_string.txt", "normal_reference_string.txt"}; // 
    vector<string> algorithmName = {"FIFO", "SecondChance", "ESC", "LRU", "LRU-LFU", "ARB", "Optimal"}; //  

    // generate three test reference strings:
    ReferenceStringGenerator generator(dataSize, referenceSize, dirtyRate);
    // Random: Arbitrarily pick [1, 20] continuous numbers for each reference.
    generator.UniformRandom(referenceRange, "uniform_reference_string.txt");
    // Locality: Simulate function calls. Each function call may refer a subset of 1/30~1/20 string
    generator.LocalityUniformRandom(referenceRange, subsetRateA, subsetRateB, "locality_reference_string.txt");
    
    generator.NormalRandom(referenceSize / 2, referenceSize / setSize, "normal_reference_string.txt");
    generator.ExponentialRandom(lambda, "exponential_reference_string.txt");

    // init performance report and page replacement objects;
    PerformanceReport performance;
    PageReplacement pageReplacement(memorySize[0], fileName[0]);
    
    // Execute different algoritms on each tests with different number of frames.
    for (int i = 0; i < fileName.size(); ++i) {
        pageReplacement.setFileName(fileName[i]);
        cout << "The reference string file is: " << fileName[i] << endl;
        cout << "The size of data: " << pageReplacement.getFileSize() << endl;
        cout << endl;

        for (int j = 0; j < memorySize.size(); ++j) {

            cout << "The number of frames: " << memorySize[j] << endl;
            pageReplacement.setMemorySize(memorySize[j]);
            cout << endl;
            
            performance = pageReplacement.FIFO();
            performance.printReport();
            performance.writeCsvReport(fileName[i], memorySize[j]);
            
            performance = pageReplacement.SecondChance();
            performance.printReport();
            performance.writeCsvReport(fileName[i], memorySize[j]);

            performance = pageReplacement.EnhancedSecondChance();
            performance.printReport();
            performance.writeCsvReport(fileName[i], memorySize[j]);

            performance = pageReplacement.LRU();
            performance.printReport();
            performance.writeCsvReport(fileName[i], memorySize[j]);

            performance = pageReplacement.LRU_LFU();
            performance.printReport();
            performance.writeCsvReport(fileName[i], memorySize[j]);

            performance = pageReplacement.ARB(setSize);
            performance.printReport();
            performance.writeCsvReport(fileName[i], memorySize[j]);

            performance = pageReplacement.Optimal();
            performance.printReport();
            performance.writeCsvReport(fileName[i], memorySize[j]);
        }
    }

    return 0;
}