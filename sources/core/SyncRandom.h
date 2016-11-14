#pragma once

#include <vector>
#include <random>

typedef std::ranlux24 RandomGenerator;

class SyncRandom
{
public:
    SyncRandom();
    unsigned int GetRand();
    inline int RandomShuffle(int v)
    {
        return static_cast<int>(GetRand() % v);
    }
    void SetRand(unsigned int seed, unsigned int calls_counter);
    unsigned int GetSeed();
    unsigned int GetCallsCounter();
private:
    unsigned int seed_;
    unsigned int calls_counter_;
    RandomGenerator generator_;
};
