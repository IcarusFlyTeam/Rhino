#include "Arduino.h"
#include "controllo_innesco.h"

controllo_innesco::controllo_innesco(int pin_clk, int  pin_cs, int pin_enable)
{
  //definisco tutti i vari pin e le loro configurazioni
 pinMode(pin_clk,OUTPUT);
 pinMode(pin_cs, OUTPUT);
 pinMode(pin_enable, OUTPUT);
}

 void controllo_innesco::sel_action(int sel, int chipsel)   //funzione per selezionare azione
 {
    int i;
        
    for(i=0; i<sel+1 ; i++)  //ciclo che mi serve per dare tanti clk quanto � il numero che voglio usare
         {
          digitalWrite(pin_clk, HIGH);  //MI CREA ONDA QUADRA DI PERIODO 200ms -> f=5Hz posso metterla pi� veloce anche
          delay(100);
          digitalWrite(pin_clk, LOW);
          delay(100);
          }

   if (chipsel=1)     // una volta che ho ottenuto il numero desiderato attivo il chip su cui voglio operare
        digitalWrite(pin_cs, HIGH);
   else
        digitalWrite(pin_cs, LOW);
   
 }

 void controllo_innesco::abilita_enable()   //funzione per abilitare enable
  {
    digitalWrite(pin_enable, HIGH); 
  }

  
 void controllo_innesco::disabilita_enable()  //funzione per disabilitare enable
  {
    digitalWrite(pin_enable, LOW); 
  }
