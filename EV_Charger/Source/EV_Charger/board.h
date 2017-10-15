/*****************************************************************************
*
*  board.h - FRAM board definitions
*  Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#ifndef BOARD_H
#define	BOARD_H

#define SPI_CS_PORT_OUT    P2OUT
#define SPI_CS_PORT_DIR    P2DIR
#define SPI_CS_PORT_SEL    P2SEL
#define SPI_CS_PORT_SEL2   P2SEL2
#define SPI_CS_PIN         BIT0

#define SPI_IRQ_PORT_DIR   P1DIR
#define SPI_IRQ_PORT_SEL   P1SEL
#define SPI_IRQ_PORT_SEL2  P1SEL2
#define SPI_IRQ_PIN        BIT4

#define WLAN_EN_PORT_OUT   P1OUT
#define WLAN_EN_PORT_DIR   P1DIR
#define WLAN_EN_PORT_SEL   P1SEL
#define WLAN_EN_PORT_SEL2  P1SEL2
#define WLAN_EN_PIN        BIT3



void pio_init();

long ReadWlanInterruptPin(void);
void WlanInterruptEnable();
void WlanInterruptDisable();
void WriteWlanPin( unsigned char val );
void initClk(void);
void startCharge();
void stopCharge();

void configureTimerPWM();
void initADC();
void configureWatchdog();
void Timeout();
void resetWDT();
#endif



