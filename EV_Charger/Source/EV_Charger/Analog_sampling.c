#include "ControlPilot.h"
#include "Main.h"
#include "Wattmeter.h"
#include "Charger_States.h"
#include "Analog_sampling.h"
#include "ControlPilot.h"
#include <msp430.h>



#pragma vector=SD24_VECTOR
__interrupt void SD24AISR(void)
{
  
  unsigned short voltage,current;
  unsigned short Pilot;
  
  switch (SD24IV)
  {
  case 2:                       // SD24MEM Overflow
    break;
  case 4:                       // SD24MEM0 IFG
    break;
  case 6:                       // SD24MEM1 IFG  
   
    voltage = SD24MEM0;         // Save CH0 results (clears IFG)
    current = SD24MEM1;         // Save CH1 results (clears IFG)
     if( (P1OUT& BIT0)==BIT0)
    {       
       calculatePower(voltage, current); 
    }
      
    
    break;
    
  case 8:                       // SD24MEM2 IFG   
    

  
    if(TACCR2==240)
    {
        Pilot=SD24MEM2;
       
        
        TACCR2=7760;
        CheckPilot(Pilot);
         
        
        Charger_States();
    }
    else
    {
        Pilot=SD24MEM2;
        TACCR2=240;
        //Verify Pilot_L
        if(((Pilot<39493)|(Pilot>43500))&(TACCTL1 = OUTMOD_6))   //-13V<Pilot_L<-11V & Pilot ON
            {
              Pilot_State='G';
              Pilot_Num=0;
              return;
            }
       
       
    }
  //P1OUT &= ~BIT6;
    
    break;
  }
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void SyncAnalogSampling (void)
{  

  switch (TAIV)
  {
  case 4:                       // TACCR2
     // P1OUT |= BIT6;
     SD24CCTL2 |=SD24SC;    //Triger sampling of Control Pilot
    break;
          
  }
}