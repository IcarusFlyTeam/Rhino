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
  
  Serial.print("Lunghezza mm ");
  Serial.println(pot.leggiPotenziometro());
  delay(100); 
}
