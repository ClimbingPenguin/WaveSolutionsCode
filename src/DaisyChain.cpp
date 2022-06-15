#include "DaisyChain.hpp"
#include <Arduino.h>
#include <SPI.h>

DaisyChain::DaisyChain(int csPin)
{
    cs = csPin;
    numberOfChips = 0;
    numberOfPots = 0;

    //ensure the cs pin is configured properly
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
}

DaisyChain::~DaisyChain()
{

}

void DaisyChain::add(Digipot* pot)
{
    //decide if the pot gets added to pot1 or pot2
    if((numberOfPots % 2) == 0)
    {
        //use pot1 on new chip
        numberOfChips += 1;
        allChips[numberOfChips-1].pot1 = pot;
    }
    else
    {
        //use pot2
        allChips[numberOfChips-1].pot2 = pot;
    }
    
    numberOfPots += 1;

}

void DaisyChain::printValuesInChain()
{
    Serial.print("The values in the chain of cs pin ");
    Serial.print(cs);
    Serial.println(".");
    for (int i = 0; i < numberOfChips; i++)
    {
        Serial.print("Value chip ");
        Serial.print(i);
        Serial.print("\n    pot0: ");
        allChips[i].pot1->print();
        if(i != (numberOfChips-1) || (numberOfPots % 2) == 0)
        {
            Serial.print("\n    pot1: ");
            allChips[i].pot2->print();
        }
        Serial.println("");
    }
    Serial.println("");
}

void DaisyChain::WriteSPI()
{
    //start writing to spi
    //cs pin low
    digitalWrite(cs, LOW);

    //write all pot0 of the chain
    for(int i = numberOfChips-1; i >= 0; i--)
    {
        //command
        Serial.printf("Data of chip %d, pot0 = %d\n", i, allChips[i].pot1->value());
        SPI.transfer(0b11011101);   //write pot0
        SPI.transfer(allChips[i].pot1->value());
        //data byte
    }
    digitalWrite(cs, HIGH);
    delay(1);//wait for the chips to process
    
    Serial.println("");

    //cs pin low
    digitalWrite(cs, LOW);

    //write all pot1 of the chain
    for(int i = numberOfChips-1; i >= 0; i--)
    {
        if(i != (numberOfChips-1) || (numberOfPots % 2) == 0)
        {
            //command
            Serial.printf("Data of chip %d, pot1 = %d\n", i, allChips[i].pot2->value());
            SPI.transfer(0b11011111);   //write pot1
            SPI.transfer(allChips[i].pot2->value());
        }
    }
    digitalWrite(cs, HIGH);
    delay(1);

}