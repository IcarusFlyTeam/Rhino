#ifndef POTENZIOMETRO_H
#define POTENZIOMETRO_H

#include "Arduino.h"
#define in_min 0
#define in_max 1023
#define out_min 0
#define out_max 100

class Potenziometro
{
    public:
        Potenziometro(int pin);
        float leggiPotenziometro(int potVal);
    private:
        int potPin;
};

#endif // POTENZIOMETRO_H
