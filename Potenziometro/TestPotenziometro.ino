#include "Potenziometro.h"
#define potPin A0
int potVal;
Potenziometro pot(potPin);

void setup() {
  Serial.begin(9600);
  potVal = -1;
}

void loop() {
  float lunghezza;
  potVal = analogRead(potPin);
  lunghezza = pot.leggiPotenziometro(potVal);
  Serial.print("Lunghezza mm ");
  Serial.println(lunghezza);
  delay(100); 
}
