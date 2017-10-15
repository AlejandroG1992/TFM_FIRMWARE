#include "Analog_sampling.h"
#include "ControlPilot.h"
#include "Listener.h"
#include "SmartConfig.h"
#include "Wattmeter.h"
#include "Wifi.h"
#include "Main.h"
#include <msp430.h>

//***********************PILOT AND POWER MEAS**********************************
//#define KP 0.0000094432777311898429285396229137072     //Constante de proporcionalidad entre la energía real (Wh) y el valor calculado

//******************************************************************************




void calculatePower(unsigned short voltage, unsigned short current)
{    
      signed long long longVoltage,longCurrent,power; 
      //P1OUT |= (BIT1);
   
      
      if(voltage<32768)
      { 
        longVoltage=voltage;
      }
      else
      {
        longVoltage=0xFFFFFFFFFFFF0000 | ((signed long) voltage);
      }

      if(current<32768)
      {

        longCurrent=current;
        
         
      }
      else
      {
        longCurrent=0xFFFFFFFFFFFF0000 | ((signed long) current);
        
      }
      power=longVoltage*longCurrent;
      powerSummation = powerSummation + power;   
     //  P1OUT &= ~(BIT1);
       
}

