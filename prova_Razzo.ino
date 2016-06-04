#include "controllo_innesco.h"

int selection=3;
int cs=1;

void setup() 
{
}

void loop() 
{
  controllo_innesco control(8,9,10);  //creo oggetto control di tipo controllo_innesco (in cui gli passo i pin del clk del chip select e dell enable)
  control.sel_action(selection,cs);  //accedo a funzione sel_action che mi permette di passargli il numero che voglio e il chip a cui voglio accedere


  control.abilita_enable();  //funzione per abilitare enable
  

}
