#include <msp430.h>
#include "wlan.h" 
#include "evnt_handler.h"    // callback function declaration
#include "nvmem.h"
#include "socket.h"
#include "netapp.h"
#include "board.h"

//*****************************************************************************
//
//! pio_init
//!
//! @param  none
//!
//! @return none
//!
//! @brief  Initialize the board's I/O
//
//*****************************************************************************    

void pio_init()
{
	P1SEL =0xE0;
        P1SEL2 =0x00;
        P1DIR=0x0F;
        P1OUT= 0x01;
        P2SEL=0xC0;
        P2SEL2=0x00;
        P2DIR=0x01;
        P2OUT=0x01;
          
	__delay_cycles(120000);
	
}
//*****************************************************************************
//
//! ReadWlanInterruptPin
//!
//! @param  none
//!
//! @return none
//!
//! @brief  return wlan interrup pin
//
//*****************************************************************************

long ReadWlanInterruptPin(void)
{
	return (P1IN & BIT4);
}

//*****************************************************************************
//
//! WlanInterruptEnable
//!
//! @param  none
//!
//! @return none
//!
//! @brief  Enable waln IrQ pin
//
//*****************************************************************************

void WlanInterruptEnable()
{
	__bis_SR_register(GIE);
	P1IES |= BIT4;
	P1IE |= BIT4;
}

//*****************************************************************************
//
//! WlanInterruptDisable
//!
//! @param  none
//!
//! @return none
//!
//! @brief  Disable wlan IrQ pin
//
//*****************************************************************************

void WlanInterruptDisable()
{
	P1IE &= ~BIT4;
}

//*****************************************************************************
//
//! WriteWlanPin
//!
//! @param  val value to write to wlan pin
//!
//! @return none
//!
//! @brief  write value to wlan pin
//
//*****************************************************************************

void WriteWlanPin( unsigned char val )
{
	if (val)
	{
		P1OUT |= BIT3;	
	}
	else
	{
		P1OUT &= ~BIT3;
	}
}

//*****************************************************************************
//
//! initClk
//!
//!  @param  None
//!
//!  @return none
//!
//!  @brief  Init the device with 16 MHz DCOCLCK.
//
//*****************************************************************************
void initClk(void)
{
	int z;
        BCSCTL1 = XTS;
	BCSCTL2 |= SELM1 + SELS ;	                //Set SMCLK as XT2CLK when XT2 oscillator present on-chip	
	BCSCTL3 |= XT2S1;	                        //3- to 16-MHz crystal or resonator
        
        __delay_cycles(200000);	
        
        IFG1 &= ~OFIFG;                           // Clear OSCFault flag
        for (z = 50; z > 0; z--)   
        {
            while (OFIFG&IFG1);                   // OSCFault flag still set?
        }
}

void startCharge()
{
    P1DIR |= BIT0;
    P1OUT |= BIT0;
}

void stopCharge()
{
    P1OUT &= ~BIT0;
}


//***********************PILOT AND POWER MEAS**********************************

void configureTimerPWM()
{   
      P1DIR |= BIT1;     
      P1SEL |= BIT1;
      P1SEL2 &= ~(BIT1);
      
      TA0CCR0 = 8000;                           // 1KHz signal
      TACCTL1 = OUTMOD_6;
      TA0CCR1 = 8000*(0.5);                     // Duty cycle
      TACTL = MC_1 + TASSEL_2;   
}
 
void initADC()
{    
      volatile unsigned int i;
      SD24CTL = SD24REFON+SD24SSEL_1;
      SD24CCTL0 = SD24SNGL+SD24GRP+SD24DF;
      SD24CCTL1 = SD24SNGL+SD24GRP+SD24DF;
      SD24CCTL2 = SD24SNGL+SD24IE+SD24DF; 
    
      for (i = 0; i < 0x3600; i++);             // Delay for 1.2V ref startup
}

void configureWatchdog()
{
    WDTCTL = WDTPW + WDTTMSEL + WDTIS1;
    IE1 |= WDTIE;
}

void disableWatchdogTimer()
{
    IE1 &= ~(WDTIE);
}

// restart of the MSP430.  
void restartMSP430()
{	
	WDTCTL=0;	                        // Ilegal write generate a reset
}
