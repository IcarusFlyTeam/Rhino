#ifndef CONTROLLO_INNESCO_H
#define CONTROLLO_INNESCO_H


class controllo_innesco
{
    public:
        controllo_innesco(int pin_clk, int  pin_cs, int pin_enable);
        void sel_action(int sel,int chipsel);
        void abilita_enable();
        void disabilita_enable();
       
    private:
     int pin_clk, pin_cs, pin_enable;
};

#endif // CONTROLLO_INNESCO_H
