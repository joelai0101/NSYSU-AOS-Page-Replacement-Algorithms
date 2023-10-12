#include "referenceString.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <random>
#include <string>

using namespace std;

ReferenceStringGenerator::ReferenceStringGenerator(
    const int p_dataSize, const int p_referenceSize, const double p_dirtyRate)
    : dataSize(p_dataSize), 
    referenceSize(p_referenceSize),
    dirtyRate(p_dirtyRate), 
    generator(rd()),
    referenceDst(1, referenceSize), 
    probabilityDst(0, 1) {}

void ReferenceStringGenerator::UniformRandom(const int p_referenceRange, const string &fileName) {
    uniform_int_distribution<int> rangeDst(1, p_referenceRange);
    vector<pair<int, int>> referenceString;
    int p_dataSize = dataSize;

    do {
        const int referenceHead = referenceDst(generator);
        const int range = min({rangeDst(generator), p_dataSize, referenceSize - referenceHead});

        for (int i = 0; i < range; ++i) {
            int dirtyBit = probabilityDst(generator) <= dirtyRate ? 1 : 0;
            referenceString.emplace_back(referenceHead + i, dirtyBit);
        }

        p_dataSize -= range;
    } while (p_dataSize > 0);

    GenerateStringFile(referenceString, fileName);
}

void ReferenceStringGenerator::LocalityUniformRandom(const int p_referenceRange, const double subsetRateA, const double subsetRateB, const string &fileName) {
    vector<pair<int, int>> referenceString;
    int p_dataSize = dataSize;
    uniform_int_distribution<int> rangeDst(1, p_referenceRange);
    uniform_int_distribution<int> subsetDst(dataSize * subsetRateA, dataSize * subsetRateB);
        
    do{
        const int referenceHead = referenceDst(generator);
        const int range = rangeDst(generator);
        uniform_int_distribution<int> localDst(referenceHead, min(referenceHead + range, referenceSize));
        const int subsetSize = min(subsetDst(generator), p_dataSize);

        for (int i = 0; i < subsetSize; ++i) {
            int dirtyBit = probabilityDst(generator) <= dirtyRate ? 1 : 0;
            int ref = localDst(generator);
            referenceString.emplace_back(ref, dirtyBit);
        }

        p_dataSize -= subsetSize;
    } while (p_dataSize > 0);

    GenerateStringFile(referenceString, fileName);
}

void ReferenceStringGenerator::NormalRandom(const int mean, const int standardDeviation, const string &fileName) {
    normal_distribution<float> referenceDst(mean, standardDeviation);
    
    int p_dataSize = dataSize;
    ofstream file(fileName);
    while (p_dataSize--) {
        int ref = 0;
        while (ref < 1 || ref > referenceSize) {ref = referenceDst(generator);}
        const int dirtyBit = probabilityDst(generator) <= dirtyRate ? 1 : 0;
        file << ref << " " << dirtyBit << endl;
    }
    file.close();
}

void ReferenceStringGenerator::ExponentialRandom(const double lambda, const string& fileName) {
    exponential_distribution<float> referenceDst(lambda);

    int p_dataSize = dataSize;
    ofstream file(fileName);
    while (p_dataSize--) {
        int ref = 0;
        while (ref < 1 || ref > referenceSize) {
            ref = static_cast<int>(referenceDst(generator));
        }
        const int dirtyBit = probabilityDst(generator) <= dirtyRate ? 1 : 0;
        file << ref << " " << dirtyBit << endl;
    }
    file.close();
}

void ReferenceStringGenerator::PoissonRandom(const double lambda, const string& fileName) {
    poisson_distribution<int> referenceDst(lambda);

    int p_dataSize = dataSize;
    ofstream file(fileName);
    while (p_dataSize--) {
        int ref = 0;
        while (ref < 1 || ref > referenceSize) {
            ref = referenceDst(generator);
        }
        const int dirtyBit = probabilityDst(generator) <= dirtyRate ? 1 : 0;
        file << ref << " " << dirtyBit << endl;
    }
    file.close();
}

void ReferenceStringGenerator::GenerateStringFile(vector<pair<int, int>> referenceString, const string &fileName) {
    // Write the reference string and dirty bit to the output file.
    ofstream file(fileName);
    for (const auto& entry : referenceString)
        file << entry.first << " " << entry.second << std::endl;
    file.close();
}