#include "nvmem.h"
#include "wlan.h"
#include "board.h"
#include "ControlPilot.h"
#include "Analog_sampling.h"
#include "ControlPilot.h"
#include "Listener.h"
#include "FlashMemory.h"
#include "Charger_States.h"
#include "Wattmeter.h"
#include "SmartConfig.h"
#include "Main.h"
#include "Wifi.h"
#include <msp430.h>
#include <string.h>
#include <stdio.h>



void Charger_States()
{
  
  switch(Charger_State)
  {
    case Not_Connected:
      ControlPilot_OFF();
      stopCharge();
      if((Pilot_State=='B')&(Pilot_Num>2000))
         Charger_State=Connected;
      if((Pilot_State=='E')&(Pilot_Num>2000))
         Charger_State=SmartConfig_Inic;
      if(((Pilot_State=='G')|(Pilot_State=='C')|(Pilot_State=='D'))&(Pilot_Num>50))
         Charger_State=Error;    
      break;
      
    case Connected:
      ControlPilot_ON();
      stopCharge();
      if((Pilot_State=='A')&(Pilot_Num>100))
         Charger_State=Not_Connected;
      if(((Pilot_State=='C')|(Pilot_State=='D'))&(Pilot_Num>3000))
         Charger_State=Charging;
      if((Pilot_State=='G')&(Pilot_Num>50))
         Charger_State=Error;  
      break;
      
    case Charging:
      ControlPilot_ON();
      startCharge();
      if((Pilot_State=='A')&(Pilot_Num>50))
         Charger_State=Not_Connected;
      if((Pilot_State=='B')&(Pilot_Num>50))
         Charger_State=Connected;
      if((Pilot_State=='G')&(Pilot_Num>50))
         Charger_State=Error;  
      break;
  
    case Error:      
      ControlPilot_OFF();
      stopCharge();
      if((Pilot_State=='E')&(Pilot_Num>2000))
         Charger_State=SmartConfig_Inic;
      break;
      
    case SmartConfig_Inic:
      Charger_State=SmartConfig_Exec;
      TA0CCR1=0;
      powerSummation=0;
      set_Registers(0x00);
      WDTCTL=0;    //Reset
      break;
      
    case SmartConfig_Exec:  
      ControlPilot_OFF();
      stopCharge();  
      TACCTL2&= ~CCIE;
      WDTCTL = WDTPW + WDTHOLD;

      
           
      StartSmartConfig();
      
      configureWatchdog(); 
      Charger_State=SmartConfig_Assign;
      set_Registers(0x00);
      TACCTL2|= CCIE;   
      ControlPilot_ON();
      initDriver();
     //wlan_ioctl_set_connection_policy(DISABLE, ENABLE, ENABLE);
      sendSmartConfigComplete();    
      break;
      
    case SmartConfig_Assign:
      if((Pilot_State=='E')&(Pilot_Num>2000))
         Charger_State=SmartConfig_Inic; 
      break;  
  }
  
  
  
  
}  


void sendSmartConfigComplete()
{
    char i=0;
    int res;
    char answer[8]={1,0,0,0,0,0,0,0};
       
    do
    {
    
      __delay_cycles(120000);
      res=wlan_ioctl_statusget();
    }while( (res!=WLAN_STATUS_CONNECTED));
    
    sendMessage(answer);
    while((i<10)&(Charger_State==SmartConfig_Assign))
    {    
    sendMessage(answer);
    receiveMessage();
    checkMessage();
    i++;
    }
     
}