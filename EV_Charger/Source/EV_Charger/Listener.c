#include "Wifi.h" 
#include "Wattmeter.h" 
#include <msp430.h>
#include "board.h"
#include "Analog_sampling.h"
#include "ControlPilot.h"
#include "Charger_States.h"
#include "Listener.h"
#include "FlashMemory.h"
#include "SmartConfig.h"
#include "Wattmeter.h"
#include "Wifi.h"
#include "nvmem.h"
#include "Main.h"
#include <msp430.h>



void checkMessage()
{     
  char answer[8]={0,0,0,0,0,0,0,0};
  answer[1]=read_Identifier();
           
  switch (((char)pucCC3000_Rx_Buffer[0])) {
       
        
        //Vinculate charger
        case 0:   //Confirm Vinculation, Save Charger Identifier
          
          if(Charger_State==SmartConfig_Assign)
          {
             answer[0]=0;
             answer[1]=pucCC3000_Rx_Buffer[1];
             
             sendMessage(answer);  //Confirm Vinculation    
            // __delay_cycles(1000000); 
             Charger_State=Not_Connected;
             set_Registers(pucCC3000_Rx_Buffer[1]);
          }
          break;
        case 1:  //Nothing
          break;
          
        case 2: //Read Active Energy Consumption 
                
                answer[0]=2;
                
                
                answer[7]= (powerSummation>>56)&(0xFF);                
                answer[6] = (powerSummation>>48)&(0xFF);
                answer[5] = (powerSummation>>40)&(0xFF);
                answer[4] = (powerSummation>>32)&(0xFF);
                answer[3] = (powerSummation>>24)&(0xFF);
                answer[2] = (powerSummation>>16)&(0xFF);
                
               
                sendMessage(answer);
          break;
        case 3:   //Read Reactive Energy Consumption   
          
          break;
        case 4:  //Read Average Power   
          
          break;
          
        case 5: //Read Maximum Power
          
          break;
        
        case 6:   //Reset Wattmeter
                answer[0]=6;
                powerSummation=0;
                sendMessage(answer);  
          break; 
          
        case 7:   //Read car status
                WDT_counter=0;
                answer[0]=7;                
                answer[2] = Charger_State;
                sendMessage(answer);  
          break;
        case 8: //Restore from Error State 
                Charger_State=Not_Connected;
                answer[0]=8;            
                sendMessage(answer);
          break;   
        case 9:  //Set Max Power
                answer[0]=9;
                
                answer[7]=pucCC3000_Rx_Buffer[1];
                if((unsigned short) pucCC3000_Rx_Buffer[1]<3)
                      TACCR1=8000;
                else
                      TA0CCR1 = 80*((unsigned short) pucCC3000_Rx_Buffer[1]); 
                                
                sendMessage(answer);  
          
          break; 
         case 10:  //Ping to Identifier IP Charger (Broadcast) or check connection
             if(pucCC3000_Rx_Buffer[1]==read_Identifier())
             {
                answer[0]=10;  
                
                sendMessage(answer);  
             }
          break;
          
          
  default:
    break;
  }
}



