#include <msp430.h>
#include "wlan.h" 
#include "netapp.h" 
#include "evnt_handler.h"    // callback function declaration
#include "FlashMemory.h"
#include "nvmem.h"
#include "socket.h"
#include "netapp.h"
#include "board.h"
#include "Main.h"
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
	P1SEL =0xE0;    //Configure p1.5, p1.6 and p1.7 for MO, MI and CLK for SPI communications 
        P1SEL2 =0x00;
        
        P1DIR=0x0F;     //Configure P1.0 as SW output, P1.1 as SPI_CS output, p1.2 PWM output, p1.3 RF_PWR_EN output
        P1OUT= 0x00;
        
        P2SEL=0xC0;  //Configure p2.6 & p2.7 for XT2 input
        P2SEL2=0x00;
        
        P2DIR=0x01;   //Configure p2.0 as output for CS
        P2OUT=0x01;
          
	
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
	//XT2=8 MHz for SMCLK & LP for ACLK
        BCSCTL1 = XTS;
	BCSCTL2 |= SELM1 + SELS ;	                //Set SMCLK as XT2CLK when XT2 oscillator present on-chip	
	BCSCTL3 |= XT2S1;	                        //3- to 16-MHz crystal or resonator
        
         //LP=12 kHz
        BCSCTL1 = DIVA1+DIVA0;
        BCSCTL3 |= LFXT1S1;
              
        do
        {
          IFG1 &= ~OFIFG;
        } while((IFG1 & OFIFG));
        
       
        
}

void startCharge()
{
    P1DIR |= BIT0;
    P1OUT |= BIT0;
    //SD24CCTL1 |= SD24IE;
    SD24CCTL1 &= ~SD24SNGL;
    SD24CCTL1 |=SD24SC;    //Triger sampling
}

void stopCharge()
{
    P1OUT &= ~BIT0;
    //SD24CCTL1 &= ~SD24IE;
    SD24CCTL1 |= SD24SNGL;
    
    
}


//***********************PILOT AND POWER MEAS**********************************

void configureTimerPWM()
{   
      
      P1DIR |= BIT1;     
     // P1SEL &= ~(BIT1);
     // P1SEL2 &= ~(BIT1);
      
      
      TA0CCR0 = 8000;                           // 1KHz signal           
      TACCTL1 = OUTMOD_0;
      P1OUT |= BIT1;
      TACTL = MC_1 + TASSEL_2;   
      //Configure to synchronize with sampling Pilot
      
      TACCR2=50;
}
 
void initADC()
{    //Use SMCLK , Ref 1.2V , 2 complement
      volatile unsigned int i;
      SD24CTL = SD24REFON+SD24SSEL_1+SD24DIV_1;  //fM<3.3 MHz, SMCLK /2
      
      //Channel0=voltage and Channel1=current
      SD24CCTL0 = SD24DF+SD24GRP;  //2s complement, Grouped
      SD24CCTL1 = SD24DF+SD24IE+SD24XOSR;  // 512 Oversampling-> 2*512/8E6=128 us sampling period
            
      //Channel2=Control Pilot  and synchonized with Control Pilot (TACCR2)
      SD24CCTL2 = SD24SNGL+SD24DF+SD24OSR1+SD24OSR0; // 
    
      for (i = 0; i < 0x3600; i++);             // Delay for 1.2V ref startup
      
      SD24CCTL1 |=SD24SC;    //Triger sampling 
}

void configureWatchdog()
{
    WDTCTL = WDTPW + WDTTMSEL+WDTSSEL;
   
    IE1 |= WDTIE;
    __bis_SR_register(GIE);

}

char WDT_counter;

#pragma vector=WDT_VECTOR
__interrupt void WatchDogTimer (void)
{  
  
    if(WDT_counter==3)
    {
          //Try to reconnect
          set_Registers(read_Identifier());        
          WDTCTL = WDTHOLD;	       // Reset if not connected
         
    }
  
    if(WDT_counter>6)
    {
          //System reset
          set_Registers(read_Identifier());        
          WDTCTL = WDTHOLD;	       // Reset if not connected
         
    }
    
    WDT_counter++;
  
}



// restart of the MSP430.  
void resetWDT()
{	    
        //WDT_counter=0;     
	WDTCTL = WDTPW + WDTTMSEL+WDTCNTCL;
}

// Timeout configuration  

void Timeout()
{
      unsigned long aucDHCP       = 14400;
        unsigned long aucARP        = 14400;
        unsigned long aucKeepalive  = 14400;
        unsigned long aucInactivity = 5;

        netapp_timeout_values(&aucDHCP, &aucARP, &aucKeepalive, &aucInactivity);
}

