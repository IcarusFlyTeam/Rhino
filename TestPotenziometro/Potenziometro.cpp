#include "Potenziometro.h"

Potenziometro::Potenziometro(int pin)
{
    potPin = pin;
}

float Potenziometro::leggiPotenziometro(int potVal) {
    potVal = analogRead(potPin);
    return ((potVal - in_min)*(float)(out_max - out_min))/(float)(in_max - in_min) + (float)out_min;
}
