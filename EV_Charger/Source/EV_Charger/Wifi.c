#include "wlan.h" 
#include "evnt_handler.h"    
#include "nvmem.h"
#include "socket.h"
#include "netapp.h"
#include "spi.h"
#include "spi_version.h"
#include "board.h"
#include "Wifi.h"
#include "application_version.h"
#include "host_driver_version.h"
#include <msp430.h>
#include "security.h"
#include "main.h"

#define PALTFORM_VERSION                               (1)


volatile unsigned long ulSmartConfigFinished, ulCC3000Connected,ulCC3000DHCP, OkToDoShutDown, ulCC3000DHCP_configured;
volatile unsigned char ucStopSmartConfig;
volatile long ulSocket;


// Simple Config Prefix
const char aucCC3000_prefix[] = {'T', 'T', 'T'};
//unsigned char printOnce = 1;
const char digits[] = "0123456789";

//static char *datos;
 // The family is AF_INET
enum 
{
    ARP_INIT = 1,
    ARP_IN_PROGRESS = 2,
    ARP_DONE
} ulArpStatus;

sockaddr tSocketAddr;

//******************************************************************************


__no_init unsigned char pucCC3000_Rx_Buffer[CC3000_APP_BUFFER_SIZE + CC3000_RX_BUFFER_OVERHEAD_SIZE];



//! @brief  Convert integer to ASCII in decimal base
unsigned short itoa(char cNum, char *cString)
{
    char* ptr;
    char uTemp = cNum;
    unsigned short length;

    // Value 0 is a special case
    if (cNum == 0)
    {
        length = 1;
        *cString = '0';
        
        return length;
    }

    // Find out the length of the number, in decimal base
    length = 0;
    while (uTemp > 0)
    {
        uTemp /= 10;
        length++;
    }

    // Do the actual formatting, right to left
    uTemp = cNum;
    ptr = cString + length;
    while (uTemp > 0)
    {
        --ptr;
        *ptr = digits[uTemp % 10];
        uTemp /= 10;
    }

    return length;
}

//! @brief  Convert nibble to hexdecimal from ASCII
unsigned char atoc(char data)
{
    unsigned char ucRes;

    if ((data >= 0x30) && (data <= 0x39))
    {
        ucRes = data - 0x30;
    }
    else
    {
        if (data == 'a')
        {
            ucRes = 0x0a;;
        }
        else if (data == 'b')
        {
            ucRes = 0x0b;
        }
        else if (data == 'c')
        {
            ucRes = 0x0c;
        }
        else if (data == 'd')
        {
            ucRes = 0x0d;
        }
        else if (data == 'e')
        {
            ucRes = 0x0e;
        }
        else if (data == 'f')
        {
            ucRes = 0x0f;
        }
    }

    return ucRes;
}

//! @brief  Convert 2 nibbles in ASCII into a short number
unsigned short atoshort(char b1, char b2)
{
    unsigned short usRes;

    usRes = (atoc(b1)) * 16 | atoc(b2);

    return usRes;
}

//! @brief  Convert 2 bytes in ASCII into one character
unsigned char ascii_to_char(char b1, char b2)
{
    unsigned char ucRes;

    ucRes = (atoc(b1)) << 4 | (atoc(b2));

    return ucRes;
}

//! @brief  The function returns a pointer to the driver patch: since there is  
//!         no patch (patches are taken from the EEPROM and not from the host
//!         - it returns NULL
char *sendDriverPatch(unsigned long *Length)
{
    *Length = 0;
    return NULL;
}

//! @brief  The function returns a pointer to the bootloader patch: since there   
//!         is no patch (patches are taken from the EEPROM and not from the host
//!         - it returns NULL
char *sendBootLoaderPatch(unsigned long *Length)
{
    *Length = 0;
    return NULL;
}


//! @brief  The function returns a pointer to the driver patch: since there is  
//!         no patch (patches are taken from the EEPROM and not from the host
//!         - it returns NULL
char *sendWLFWPatch(unsigned long *Length)
{
    *Length = 0;
    return NULL;
}

//! @brief  The function handles asynchronous events that come from CC3000  
//!         device and operates a LED1 to have an on-board indication

void CC3000_UsynchCallback(long lEventType, char * data, unsigned char length)
{
    if (lEventType == HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE)
    {
        ulSmartConfigFinished = 1;
        ucStopSmartConfig     = 1;  
    }

    if (lEventType == HCI_EVNT_WLAN_UNSOL_CONNECT)
    {
        ulCC3000Connected = 1;
      
        // Turn on the LED7

    }

    if (lEventType == HCI_EVNT_WLAN_UNSOL_DISCONNECT)
    {		
        ulCC3000Connected = 0;
        ulCC3000DHCP      = 0;
        ulCC3000DHCP_configured = 0;
        //printOnce = 1;
       
        // Turn off the LED7

        
        // Turn off LED5
         
    }

    if (lEventType == HCI_EVNT_WLAN_UNSOL_DHCP)
    {
        // Notes: 
        // 1) IP config parameters are received swapped
        // 2) IP config parameters are valid only if status is OK, 
        // i.e. ulCC3000DHCP becomes 1

        // Only if status is OK, the flag is set to 1 and the 
        // addresses are valid.
        if ( *(data + NETAPP_IPCONFIG_MAC_OFFSET) == 0)
        {
            sprintf((char*)pucCC3000_Rx_Buffer,"IP:%d.%d.%d.%d\f\r", data[3],data[2], data[1], data[0]);
            ulCC3000DHCP = 1;

        }
        else
        {
            ulCC3000DHCP = 0;

        }
    }

    if (lEventType == HCI_EVENT_CC3000_CAN_SHUT_DOWN)
    {
        OkToDoShutDown = 1;
    }	
    
    
    if (lEventType == HCI_EVNT_ASYNC_ARP_DONE)
    {
       ulArpStatus = ARP_DONE;
    }

    if (lEventType == HCI_EVNT_ASYNC_ARP_WAITING)
    {
       ulArpStatus = ARP_IN_PROGRESS;
    }  
}


//!  @brief  The function initializes a CC3000 device and triggers it to 
//!          start operation
int initDriver(void)
{
    ulCC3000DHCP = 0;
    ulCC3000Connected = 0;
    ulSocket = 0;
    ulSmartConfigFinished=0;
    
     
    pio_init();
    __delay_cycles(120000);
    
    init_spi();
    

    // WLAN On API Implementation
    wlan_init(CC3000_UsynchCallback, sendWLFWPatch, sendDriverPatch, sendBootLoaderPatch, ReadWlanInterruptPin, WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);


    // Trigger a WLAN device
    wlan_start(0);

    // Mask out all non-required events from CC3000
    wlan_set_event_mask(HCI_EVENT_MASK);
     // Generate the event to CLI: send a version string
    {
        char cc3000IP[50];
        char *ccPtr;
        unsigned short ccLen;
                     
        ccPtr = &cc3000IP[0];
        ccLen = itoa(PALTFORM_VERSION, ccPtr);
        ccPtr += ccLen;
        *ccPtr++ = '.';
        ccLen = itoa(APPLICATION_VERSION, ccPtr);
        ccPtr += ccLen;
        *ccPtr++ = '.';
        ccLen = itoa(SPI_VERSION_NUMBER, ccPtr);
        ccPtr += ccLen;
        *ccPtr++ = '.';
        ccLen = itoa(DRIVER_VERSION_NUMBER, ccPtr);
        ccPtr += ccLen;
        *ccPtr++ = '\f';
        *ccPtr++ = '\r';
        *ccPtr++ = '\0';
    }

    __delay_cycles(2000000); 

    ucStopSmartConfig   = 0;
    
    
     
    
    return(0);
}






//Close socket
void closeSocket()
{
    closesocket( ulSocket );       
    ulSocket = 0xFFFFFFFF;    
}

//Function send UDP message
void sendMessage(char *datos)
{
 
        SD24CCTL2 &= ~SD24IE;
        TACCTL2&= ~CCIE;

        ulArpStatus = ARP_INIT;
        
        //Open UDP socket
        ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
         // The family is AF_INET
        tSocketAddr.sa_family = AF_INET;  
        
        //PORT:4444
        tSocketAddr.sa_data[0] = 0x11;
        tSocketAddr.sa_data[1] = 0x5C; 
 
        //IP:192.168.1.128
        tSocketAddr.sa_data[2] = 0xC0;
        tSocketAddr.sa_data[3] = 0xA8;
        tSocketAddr.sa_data[4] = 0x01;
        tSocketAddr.sa_data[5] = 0x80;
        //unsigned long longitud=strlen(datos);  
        
        sendto(ulSocket, datos, 8, 0, &tSocketAddr, sizeof(sockaddr));                
       
        
       /* if(firstSend == 1)
        {   
            while(ulArpStatus == ARP_INIT || ulArpStatus == ARP_IN_PROGRESS); 
            
           firstSend = 0;
       }
       */         
       __delay_cycles(2000); 
       
        closeSocket();
        SD24CCTL2 |= SD24IE;
        TACCTL2|= CCIE;

           
}




//Function receive UDP message
void receiveMessage()
{
        SD24CCTL2 &= ~SD24IE;
        TACCTL2&= ~CCIE;
       // sockaddr tSocketAddr;
        socklen_t tRxPacketLength;  
        //Open UDP socket
    
        ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
           
        // The family is AF_INET
        tSocketAddr.sa_family = AF_INET;  
        
        //PORT:4444
        tSocketAddr.sa_data[0] = 0x11;
        tSocketAddr.sa_data[1] = 0x5C; 
 
        //IP:192.168.1.128
        tSocketAddr.sa_data[2] = 0xC0;
        tSocketAddr.sa_data[3] = 0xA8;
        tSocketAddr.sa_data[4] = 0x01;
        tSocketAddr.sa_data[5] = 0x80;
         
        //Fill data in tSocketAddr with "0"
        memset (&tSocketAddr.sa_data[2], 0, 4);  
        //Associate socket ato a port and ip address
        bind(ulSocket, &tSocketAddr, sizeof(sockaddr));   
        SD24CCTL2 |= SD24IE;
        TACCTL2|= CCIE;
        recvfrom(ulSocket, pucCC3000_Rx_Buffer, CC3000_APP_BUFFER_SIZE, 0, &tSocketAddr, &tRxPacketLength); 
        SD24CCTL2 &= ~SD24IE;
        TACCTL2&= ~CCIE;
        closeSocket();
        SD24CCTL2 |= SD24IE;
        TACCTL2|= CCIE;
      
}


