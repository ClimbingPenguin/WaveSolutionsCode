#pragma once
#include "digipot.hpp"

class DaisyChain
{
private:
    struct Chip
    {
        Digipot* pot1 = 0;
        Digipot* pot2 = 0;
    };
    int numberOfChips;
    int numberOfPots;
    int cs;
    Chip allChips[10];
public:
    DaisyChain(int csPin);
    ~DaisyChain();
    void add(Digipot* pot);
    void printValuesInChain();
    void WriteSPI();
};