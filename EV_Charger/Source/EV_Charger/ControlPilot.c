#include "Charger_Station.h"
#include "Analog_sampling.h"
#include "Main.h"
#include <msp430.h>



void CheckPilot( unsigned short Pilot_H)
{    
  
    
  //Verify Pilot_H
    if((Pilot_H<26042)&(Pilot_H>22035))   //11V<Pilot_H<13V
    {
      if(Pilot_State=='A')
      {
        if(Pilot_Num<20000)
          Pilot_Num ++;
      }
      else
        Pilot_Num=0;
      
      Pilot_State='A';
      return;
    }
    
    if((Pilot_H<20032)&(Pilot_H>16025))   //10V<Pilot_H<8V
    {
      if(Pilot_State=='B')
      {
        if(Pilot_Num<20000)
          Pilot_Num ++;
      }
      else
        Pilot_Num=0;
      
      Pilot_State='B';
      return;
    }
    
    if((Pilot_H<14022)&(Pilot_H>10016))   //6V<Pilot_H<7V
    {
      if(Pilot_State=='C')
      {
        if(Pilot_Num<20000)
          Pilot_Num ++;
      }
      else
        Pilot_Num=0;
      
      Pilot_State='C';
      return;
    }
    
    if((Pilot_H<8013)&(Pilot_H>4006))   //2V<Pilot_H<4V
    {
      if(Pilot_State=='D')
      {
        if(Pilot_Num<20000)
          Pilot_Num ++;
      }
      else
        Pilot_Num=0;
      
      Pilot_State='D';
      return;
    }
    
     if((Pilot_H<2003)|(Pilot_H>63532))   //-1V<Pilot_H<1V
    {
      if(Pilot_State=='E')
      {
       if(Pilot_Num<20000)
          Pilot_Num ++;
      }
      else
        Pilot_Num=0;
      
      Pilot_State='E';
      return;
    }
    
    if((Pilot_H<43500)&(Pilot_H>39493))   //-13V<Pilot_H<-11V
    {
      if(Pilot_State=='F')
      {
        if(Pilot_Num<20000)
          Pilot_Num ++;
      }
      else
        Pilot_Num=0;
      
      Pilot_State='F';
      return;
    }
   Pilot_Num=0;
   Pilot_State='G';
      
}

void ControlPilot_ON()  //Duty=TA0CCR1
{  
   if(TACCR1<240)
     TACCR1=8000;
   P1SEL |= BIT1;
   TACCTL1 = OUTMOD_7; 
}

void ControlPilot_OFF()
{  
   P1SEL &= ~(BIT1);
   P1OUT |= BIT1;
   TACCTL1 = OUTMOD_0;  
}