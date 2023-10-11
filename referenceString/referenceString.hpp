#ifndef __referenceString__
#define __referenceString__

#include <vector>
#include <random>

using namespace std;

class ReferenceStringGenerator {
public:
    ReferenceStringGenerator(const int p_dataSize = 200000, 
                        const int p_referenceSize = 1000, 
                        const double p_dirtyRate = 0.5);
    ~ReferenceStringGenerator() {}
    void RandomPick(const int p_referenceRange = 20, const string &fileName = "random_reference_string.txt");
    void LocalitySimulate(const int p_referenceRange, const double subsetRateA = 1.0 / 30.0, const double subsetRateB = 1.0 / 20.0, const string &fileName = "locality_reference_string.txt");
    
private:
    int referenceSize; // page Reference string: 1~1,000
    int dataSize; // Number of memory references: At least 200,000 times
    double dirtyRate; // You can use both reference and dirty bits.
    int referenceRange; // Arbitrarily pick [1, 20] continuous numbers for each reference.
    
    random_device rd; 
    mt19937 generator; // pseudorandom number generator
    uniform_int_distribution<int> referenceDst;
    uniform_real_distribution<double> probabilityDst;

    void GenerateStringFile(vector<pair<int, int>> referenceString, const string &fileName);
};

#endif // __referenceString__