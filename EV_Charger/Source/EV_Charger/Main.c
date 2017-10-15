#include "nvmem.h"
#include "wlan.h"
#include "evnt_handler.h"
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

char output_logic(void);
char state_logic(void);

char Pilot_State; 
unsigned short Pilot_Num;
signed long  long powerSummation=0;                //64 bits
char Charger_State;




void main(void) {
    WDTCTL = WDTPW + WDTHOLD;
    configureWatchdog();    //InitSystems
    pio_init();

    /*
    Charger_State=0;
    set_Registers(0);
    while(1);
    */
    
    /*
    Charger_State=3;
    powerSummation=325879;
    TA0CCR1=8425;
    set_Registers();
    read_Registers();
    
    */
    //wlan_connect("EVNET", 5);
    
  // StartSmartConfig();
   // resetWDT();
   // while(1);
   // Timeout();
   

    //Pruebas
    /*
    Charger_State=3;
    powerSummation=325879.234;
    setStatus();
    readStatus();
    setIdentifier(43); 
    */
    
  
    //Init Global Variables - Restore nonvolatil variables  (powerSummation, Charger_State, TA0CCR1)    
     read_Registers();
     
     if(Charger_State==Charging)
         startCharge();
     else
         stopCharge();
     
      if((Charger_State==Charging)|(Charger_State==Connected))
         ControlPilot_ON() ;
     else
         ControlPilot_OFF() ;
     
 
     initClk(); 
     
     //ControlPilot_ON();
     //stopCharge();
     //Pilot_State=0;
        
     SVSCTL=VLD0+VLD3+SVSON;    
     
     
     
  //Init AD, PWM and Interrupts 
      configureTimerPWM();  
      initADC();     
      
    
   // P1SEL &= ~(BIT1);
   // P1SEL2 &= ~(BIT1);
   // stopCharge();
    
    
 
    initDriver();
    //wlan_ioctl_set_connection_policy(DISABLE, ENABLE, ENABLE);
    Timeout(); 
    TACCTL2|= CCIE;
    SD24CCTL2 |= SD24IE;
    __bis_SR_register(GIE);
    
   //Not configured: Need SmartConfig
       while(Charger_State==SmartConfig_Exec){};
         
      
    while(1){
     
     receiveMessage();
     checkMessage();
    };
   
 
}

