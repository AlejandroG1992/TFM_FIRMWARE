#include "Wifi.h" 
#include "Wattmeter.h" 
#include <msp430.h>
#include "board.h"
#include "Analog_sampling.h"
#include "ControlPilot.h"
#include "Charger_States.h"
#include "Listener.h"
#include "SmartConfig.h"
#include "Wattmeter.h"
#include "Wifi.h"
#include "FlashMemory.h"
#include "nvmem.h"
#include "Main.h"
#include <msp430.h>

void set_Registers(char Identity)
{
  char  *Flash_ptr;
   __bic_SR_register(GIE); 
        
    //Borrar bloque
    FCTL2 = FWKEY + FSSEL_3 + FN4+FN2;           // SMCLK/20 for Flash Timing Generator
    Flash_ptr=(char  *) Direccion_state;
    FCTL1 = FWKEY + ERASE;                  // Set Erase bit
    FCTL3 = FWKEY;                          // Clear Lock bit
    *Flash_ptr = 0;                         // Dummy write to erase Flash segment
    while((FCTL3 & BUSY));   
  
  //Save state
  Flash_ptr=(char  *) Direccion_state;
  FCTL1 = FWKEY + WRT;
  *Flash_ptr =Charger_State;
  while((FCTL3 & BUSY));
  
  //Save Duty
  Flash_ptr=(char  *) Direccion_duty;
  FCTL1 = FWKEY + WRT;
  *Flash_ptr =(TA0CCR1)&(0xFF); ;
  while((FCTL3 & BUSY));
  
  Flash_ptr ++;
  FCTL1 = FWKEY + WRT;
  *Flash_ptr =(TA0CCR1>>8)&(0xFF);
  while((FCTL3 & BUSY));  
  
  //Save EnergyConsumption
  Flash_ptr=(char  *) Direccion_EnergyConsmpt;
  FCTL1 = FWKEY + WRT;
  *Flash_ptr =(powerSummation)&(0xFF);
  while((FCTL3 & BUSY));
  
  Flash_ptr ++;
  FCTL1 = FWKEY + WRT;
  *Flash_ptr =(powerSummation>>8)&(0xFF);
  while((FCTL3 & BUSY));  
  
   Flash_ptr ++;
  FCTL1 = FWKEY + WRT;
  *Flash_ptr =(powerSummation>>16)&(0xFF);
  while((FCTL3 & BUSY)); 
  
   Flash_ptr ++;
  FCTL1 = FWKEY + WRT;
  *Flash_ptr =(powerSummation>>24)&(0xFF);
  while((FCTL3 & BUSY)); 
  
   Flash_ptr ++;
  FCTL1 = FWKEY + WRT;
  *Flash_ptr =(powerSummation>>32)&(0xFF);
  while((FCTL3 & BUSY)); 
  
   Flash_ptr ++;
  FCTL1 = FWKEY + WRT;
  *Flash_ptr =(powerSummation>>40)&(0xFF);
  while((FCTL3 & BUSY)); 
  
   Flash_ptr ++;
  FCTL1 = FWKEY + WRT;
  *Flash_ptr =(powerSummation>>48)&(0xFF);
  while((FCTL3 & BUSY)); 
  
   Flash_ptr ++;
  FCTL1 = FWKEY + WRT;
  *Flash_ptr =(powerSummation>>56)&(0xFF);
  while((FCTL3 & BUSY)); 
  
  //Save Identity
  Flash_ptr=(char  *) Direccion_Identifier;
  FCTL1 = FWKEY + WRT;
  *Flash_ptr =Identity;
  while((FCTL3 & BUSY));
  
  
    
  FCTL3 = FWKEY + LOCK;                     // Reset LOCK bit

  
 
}


void read_Registers()
{
  char  *Flash_ptr;
  
     
  __bic_SR_register(GIE); 
  Flash_ptr=(char  *) (Direccion_state); 
  Charger_State=(*Flash_ptr);
  
  //First time
  if (Charger_State==0xFF)
  {
     Charger_State=0;
     powerSummation=0;
     TA0CCR1=8000;
     set_Registers(255);
     return;
  }

  Flash_ptr=(char  *) (Direccion_duty); 
  TA0CCR1=(*Flash_ptr);
  Flash_ptr ++;
  TA0CCR1=TA0CCR1+((*Flash_ptr)<<8);
  
  Flash_ptr=(char  *) (Direccion_EnergyConsmpt+7); 
  powerSummation=(*Flash_ptr);
  Flash_ptr --;
  powerSummation=(powerSummation<<8)+(*Flash_ptr);
  Flash_ptr --;
  powerSummation=(powerSummation<<8)+(*Flash_ptr);
  Flash_ptr --;
  powerSummation=(powerSummation<<8)+(*Flash_ptr);
  Flash_ptr --;
  powerSummation=(powerSummation<<8)+(*Flash_ptr);
  Flash_ptr --;
  powerSummation=(powerSummation<<8)+(*Flash_ptr);
  Flash_ptr --;
  powerSummation=(powerSummation<<8)+(*Flash_ptr);
  Flash_ptr --;
  powerSummation=(powerSummation<<8)+(*Flash_ptr);
  
  __bis_SR_register(GIE);
  
  
  
 }


char read_Identifier()
{
  char  *Flash_ptr;
  char Identity;
  __bic_SR_register(GIE);   
  
  Flash_ptr=(char  *) (Direccion_Identifier); 
  Identity=(*Flash_ptr);
  
  __bis_SR_register(GIE);
  
  
  return(Identity);
}





