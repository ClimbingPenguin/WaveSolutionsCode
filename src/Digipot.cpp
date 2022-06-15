//Geschreven door Wouter Peletier
//In dit bestand worden de fucnties van de Digipot class gedefinieert
#include "Digipot.hpp"
#include <Arduino.h>


Digipot::Digipot()
{
    currentVal = 0;
}
Digipot::Digipot(int data)
{
    currentVal = data;
}


Digipot::~Digipot()
{
}


int Digipot::write(int data)
{
  //the data is between 0 and 255
    if(data >= 255 || data < 0) return 0; //check of data tussen 0 en 255 is
    currentVal = data;
    return 0;
}

int Digipot::value()
{
    return currentVal;
}

void Digipot::print()
{
    Serial.print(currentVal);
}
