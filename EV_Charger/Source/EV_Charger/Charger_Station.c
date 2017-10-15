#include "board.h"
#include "ControlPilot.h"
#include "Analog_sampling.h"
#include "ControlPilot.h"
#include "Listener.h"
#include "SmartConfig.h"
#include "Wattmeter.h"
#include "Charger_Station.h"
#include "Wifi.h"
#include <msp430.h>



//*******************FSM*******************************************************
#define 	Idle            'A'
#define         Charging        'B'     
#define         SendData        'C'
#define         IPConfig        'D'

void init_system(void);
char output_logic(void);
char state_logic(void);
unsigned char present_state = Idle; 
unsigned char flag_SendData=0;
unsigned char flag_IPConfiguration=0;
unsigned char flag_EnablePilot;
unsigned char vehicleStatus=0;



void main(void) {
    WDTCTL = WDTPW + WDTHOLD;
    //InitSystems
    initClk(); 
    initDriver();
    initADC();
    configureTimerPWM();
    
    //Init Global Variables
     powerSummation =0;
     ControlPilot_OFF();
     stopCharge();
     TA0CCR1=0;  //No Power Available
          
     __bis_SR_register(GIE);
        
    while(1){
      receiveMessage();
      checkMessage();
    };
    
    
}

