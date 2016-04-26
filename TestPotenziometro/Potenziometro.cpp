#include "Potenziometro.h"

Potenziometro::Potenziometro(int pin)
{
    potPin = pin;
    pinMode(pin,INPUT);
}

float Potenziometro::leggiPotenziometro() {
    float potVal = analogRead(potPin);
    return ((potVal - in_min)*(float)(out_max - out_min))/(float)(in_max - in_min) + (float)out_min;
}
