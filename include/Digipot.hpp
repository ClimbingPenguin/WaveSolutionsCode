#pragma once

class Digipot
{
private:
    int currentVal;

public:
    Digipot();
    Digipot(int data);
    ~Digipot();
    int value();
    int write(int data);
    void print();
};