#include "wlan.h" 
#include "evnt_handler.h"    
#include "nvmem.h"
#include "socket.h"
#include "netapp.h"
#include "spi.h"
#include "spi_version.h"
#include "board.h"
#include "application_version.h"
#include "host_driver_version.h"
#include <msp430.h>
#include "security.h"

#define PALTFORM_VERSION                               (1)


//#define CC3000_APP_BUFFER_SIZE                         (5)
#define CC3000_APP_BUFFER_SIZE                         (2)
#define CC3000_RX_BUFFER_OVERHEAD_SIZE                 (20)

#define DISABLE                                        (0)
#define ENABLE                                         (1)

#define SL_VERSION_LENGTH                              (11)
#define NETAPP_IPCONFIG_MAC_OFFSET                     (20)

#define HCI_EVENT_MASK                                 (HCI_EVNT_WLAN_KEEPALIVE | HCI_EVNT_WLAN_UNSOL_INIT | HCI_EVNT_WLAN_ASYNC_PING_REPORT)

volatile unsigned long ulSmartConfigFinished, ulCC3000Connected,ulCC3000DHCP, OkToDoShutDown, ulCC3000DHCP_configured;
volatile unsigned char ucStopSmartConfig;
volatile long ulSocket;

// Simple Config Prefix
const char aucCC3000_prefix[] = {'T', 'T', 'T'};

unsigned char printOnce = 1;
char digits[] = "0123456789";

static char *datos;
//***********************PILOT AND POWER MEAS**********************************
//#define KP 0.0000094432777311898429285396229137072     //Constante de proporcionalidad entre la energía real (Wh) y el valor calculado
//#define Kv 563.3134586459090909090909090909
//#define Ki 50
//#define Kc 0.000000000335276126861572265625

static signed long long powerSummation=0;                //64 bits
static char SmartConfigControl=0;
static char controlMeas=0;
static char status=0;
//******************************************************************************

//*******************FSM*******************************************************
#define 	Idle            'A'
#define         Charging        'B'     
#define         SendData        'C'
#define         IPConfig        'D'

void init_system(void);
char output_logic(void);
char state_logic(void);

static char present_state = Idle; 
static unsigned char flag_SendData=0;
static unsigned char flag_EnableCharge=0;
static unsigned char flag_IPConfiguration=0;
static unsigned char flag_sendConsumption;
static unsigned char vehicleStatusChange=0;
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
        printOnce = 1;
       
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
}


//!  @brief  The function initializes a CC3000 device and triggers it to 
//!          start operation
int
initDriver(void)
{
    initClk();    
    pio_init();
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

    ucStopSmartConfig   = 0;
    
    initADC();
    configureTimerPWM(); 
    __delay_cycles(20000000); 
    configureWatchdog();
    
    return(0);
}

//Connect to AP
void connectAP()
{
    unsigned char pucIP_Addr[4];
    unsigned char pucIP_DefaultGWAddr[4];
    unsigned char pucSubnetMask[4];
    unsigned char pucDNS[4];
 
    //wlan_connect(WLAN_SEC_WPA2, "JAZZTEL_88BE", strlen("JAZZTEL_88BE"), NULL, "A275388F6698AF92JE95", 20);
    //wlan_connect(WLAN_SEC_UNSEC, "JAZZTEL_88BE",strlen("JAZZTEL_88BE") , NULL, NULL, 0);
    wlan_connect("JAZZTEL_88BE", strlen("JAZZTEL_88BE"));
    
    // Configure DHCP               C0A80188
    pucSubnetMask[0] = 0xFF;
    pucSubnetMask[1] = 0xFF;
    pucSubnetMask[2] = 0xFF;
    pucSubnetMask[3] = 0x0;

    
    pucIP_Addr[0] = 0xC0;
    pucIP_Addr[1] = 0xA8;
    pucIP_Addr[2] = 0x01;
    pucIP_Addr[3] = 0x82;

    pucIP_DefaultGWAddr[0] = 0xC0;
    pucIP_DefaultGWAddr[1] = 0xA8;
    pucIP_DefaultGWAddr[2] = 0x01;
    pucIP_DefaultGWAddr[3] = 0x01;

    pucDNS[0] = 0x0;
    pucDNS[1] = 0x0;
    pucDNS[2] = 0x0;
    pucDNS[3] = 0x0;

    
    netapp_dhcp((unsigned long *)pucIP_Addr, (unsigned long *)pucSubnetMask, (unsigned long *)pucIP_DefaultGWAddr, (unsigned long *)pucDNS);
                
    __delay_cycles(2000000);
    
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
        unsigned long longitud=strlen(datos);
        sockaddr tSocketAddr; 
        //Open UDP socket
        ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        
        // The family is AF_INET
        tSocketAddr.sa_family = AF_INET;  
        
        tSocketAddr.sa_data[0] = 0x11;
        tSocketAddr.sa_data[1] = 0x5C; 
        
        tSocketAddr.sa_data[2] = 0xC0;
        tSocketAddr.sa_data[3] = 0xA8;
        tSocketAddr.sa_data[4] = 0x01;
        tSocketAddr.sa_data[5] = 0x80;
                
        sendto(ulSocket, datos, longitud, 0, &tSocketAddr, sizeof(sockaddr)); 
        
}

//Function send UDP message
void sendConsumptionMessage(unsigned char *consumptionData)
{  
        __delay_cycles(2000000);
        if(flag_sendConsumption==1)
        {       
          sockaddr tSocketAddr; 
          //Open UDP socket
          ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        
          // The family is AF_INET
          tSocketAddr.sa_family = AF_INET;  
        
          tSocketAddr.sa_data[0] = 0x11;          
          tSocketAddr.sa_data[1] = 0x5C;  
        
          tSocketAddr.sa_data[2] = 0xC0;
          tSocketAddr.sa_data[3] = 0xA8;
          tSocketAddr.sa_data[4] = 0x01;
          tSocketAddr.sa_data[5] = 0x80;
          
          sendto(ulSocket, consumptionData, 2, 0, &tSocketAddr, sizeof(sockaddr));
        }
        if(flag_sendConsumption==2)
        {       
          sockaddr tSocketAddr; 
          //Open UDP socket
          ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        
          // The family is AF_INET
          tSocketAddr.sa_family = AF_INET;  
        
          tSocketAddr.sa_data[0] = 0x11;  //11            4441
          tSocketAddr.sa_data[1] = 0x59;  //59
        
          tSocketAddr.sa_data[2] = 0xC0;
          tSocketAddr.sa_data[3] = 0xA8;
          tSocketAddr.sa_data[4] = 0x01;
          tSocketAddr.sa_data[5] = 0x80;
          
          sendto(ulSocket, consumptionData, 1, 0, &tSocketAddr, sizeof(sockaddr));
        }
        if(flag_sendConsumption==3)
        {        
          sockaddr tSocketAddr; 
          //Open UDP socket
          ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        
          // The family is AF_INET
          tSocketAddr.sa_family = AF_INET;  
        
          tSocketAddr.sa_data[0] = 0x11;  //11            4441
          tSocketAddr.sa_data[1] = 0x59;  //59
        
          tSocketAddr.sa_data[2] = 0xC0;
          tSocketAddr.sa_data[3] = 0xA8;
          tSocketAddr.sa_data[4] = 0x01;
          tSocketAddr.sa_data[5] = 0x80;
          
          sendto(ulSocket, consumptionData, 1, 0, &tSocketAddr, sizeof(sockaddr));
        }
        if(flag_sendConsumption==4)
        {        
          sockaddr tSocketAddr; 
          //Open UDP socket
          ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        
          // The family is AF_INET
          tSocketAddr.sa_family = AF_INET;  
        
          tSocketAddr.sa_data[0] = 0x11;  //11            4441
          tSocketAddr.sa_data[1] = 0x59;  //59
        
          tSocketAddr.sa_data[2] = 0xC0;
          tSocketAddr.sa_data[3] = 0xA8;
          tSocketAddr.sa_data[4] = 0x01;
          tSocketAddr.sa_data[5] = 0x80;
          
          sendto(ulSocket, consumptionData, 1, 0, &tSocketAddr, sizeof(sockaddr));
        }
        if(flag_sendConsumption==5)
        {     
          sockaddr tSocketAddr; 
          //Open UDP socket
          ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        
          // The family is AF_INET
          tSocketAddr.sa_family = AF_INET;  
        
          tSocketAddr.sa_data[0] = 0x11;  //11            4441
          tSocketAddr.sa_data[1] = 0x59;  //59
        
          tSocketAddr.sa_data[2] = 0xC0;
          tSocketAddr.sa_data[3] = 0xA8;
          tSocketAddr.sa_data[4] = 0x01;
          tSocketAddr.sa_data[5] = 0x80;
          
          sendto(ulSocket, consumptionData, 1, 0, &tSocketAddr, sizeof(sockaddr));
        }
        if(flag_sendConsumption==6)
        {        
          sockaddr tSocketAddr; 
          //Open UDP socket
          ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        
          // The family is AF_INET
          tSocketAddr.sa_family = AF_INET;  
        
          tSocketAddr.sa_data[0] = 0x11;  //11            4441
          tSocketAddr.sa_data[1] = 0x59;  //59
        
          tSocketAddr.sa_data[2] = 0xC0;
          tSocketAddr.sa_data[3] = 0xA8;
          tSocketAddr.sa_data[4] = 0x01;
          tSocketAddr.sa_data[5] = 0x80;
          
          sendto(ulSocket, consumptionData, 1, 0, &tSocketAddr, sizeof(sockaddr));
        }
        if(flag_sendConsumption==7)
        {        
          sockaddr tSocketAddr; 
          //Open UDP socket
          ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        
          // The family is AF_INET
          tSocketAddr.sa_family = AF_INET;  
        
          tSocketAddr.sa_data[0] = 0x11;  //11            4441
          tSocketAddr.sa_data[1] = 0x59;  //59
        
          tSocketAddr.sa_data[2] = 0xC0;
          tSocketAddr.sa_data[3] = 0xA8;
          tSocketAddr.sa_data[4] = 0x01;
          tSocketAddr.sa_data[5] = 0x80;
          
          sendto(ulSocket, consumptionData, 1, 0, &tSocketAddr, sizeof(sockaddr));
        }
        if(flag_sendConsumption==8)
        {        
          sockaddr tSocketAddr; 
          //Open UDP socket
          ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        
          // The family is AF_INET
          tSocketAddr.sa_family = AF_INET;  
        
          tSocketAddr.sa_data[0] = 0x11;  //11            4441
          tSocketAddr.sa_data[1] = 0x59;  //59
        
          tSocketAddr.sa_data[2] = 0xC0;
          tSocketAddr.sa_data[3] = 0xA8;
          tSocketAddr.sa_data[4] = 0x01;
          tSocketAddr.sa_data[5] = 0x80;
          
          sendto(ulSocket, consumptionData, 1, 0, &tSocketAddr, sizeof(sockaddr));
        }  
        
        if(flag_sendConsumption==9)
        {        
          sockaddr tSocketAddr; 
          //Open UDP socket
          ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        
          // The family is AF_INET
          tSocketAddr.sa_family = AF_INET;  
        
          tSocketAddr.sa_data[0] = 0x11;  //11            4441
          tSocketAddr.sa_data[1] = 0x59;  //59
        
          tSocketAddr.sa_data[2] = 0xC0;
          tSocketAddr.sa_data[3] = 0xA8;
          tSocketAddr.sa_data[4] = 0x01;
          tSocketAddr.sa_data[5] = 0x80;
          
          sendto(ulSocket, consumptionData, 1, 0, &tSocketAddr, sizeof(sockaddr));
        }
}

void sendPowerMeas()
{
        IE1 &= ~(WDTIE);
        unsigned char x1 = (powerSummation>>56)&(0xFF);
        unsigned char x2 = (powerSummation>>48)&(0xFF);
        unsigned char x3 = (powerSummation>>40)&(0xFF);
        unsigned char x4 = (powerSummation>>32)&(0xFF);
        unsigned char x5 = (powerSummation>>24)&(0xFF);
        unsigned char x6 = (powerSummation>>16)&(0xFF);
        unsigned char x7 = (powerSummation>>8)&(0xFF); 
        unsigned char x8 = (powerSummation>>0)&(0xFF); 
        
        unsigned char *consumptionData;
  
        flag_sendConsumption=1;
        consumptionData="1:";
        sendConsumptionMessage(consumptionData);
        closeSocket();
        flag_sendConsumption=2;
        consumptionData=&x1;
        sendConsumptionMessage(consumptionData);
        closeSocket();
        flag_sendConsumption=3;
        consumptionData=&x2;
        sendConsumptionMessage(consumptionData);  
        closeSocket();
        flag_sendConsumption=4;
        consumptionData=&x3;
        sendConsumptionMessage(consumptionData);
        closeSocket();
        flag_sendConsumption=5;
        consumptionData=&x4;
        sendConsumptionMessage(consumptionData);
        closeSocket();
        flag_sendConsumption=6;
        consumptionData=&x5;
        sendConsumptionMessage(consumptionData); 
        closeSocket();
        flag_sendConsumption=7;
        consumptionData=&x6;
        sendConsumptionMessage(consumptionData);
        closeSocket();
        flag_sendConsumption=8;
        consumptionData=&x7;
        sendConsumptionMessage(consumptionData);
        closeSocket();
        flag_sendConsumption=9;
        consumptionData=&x8;
        sendConsumptionMessage(consumptionData);
        closeSocket();
        
        IE1 |= WDTIE;  
}

//Function send UDP message
void sendMessageSmartConfig(char *datos)
{
        unsigned long longitud=strlen(datos);
        sockaddr tSocketAddr; 
        //Open UDP socket
        ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        
        // The family is AF_INET
        tSocketAddr.sa_family = AF_INET;  
        
        tSocketAddr.sa_data[0] = 0x11;
        tSocketAddr.sa_data[1] = 0x5E; 
        
        tSocketAddr.sa_data[2] = 0xC0;
        tSocketAddr.sa_data[3] = 0xA8;
        tSocketAddr.sa_data[4] = 0x01;
        tSocketAddr.sa_data[5] = 0x80;
                
        sendto(ulSocket, datos, longitud, 0, &tSocketAddr, sizeof(sockaddr)); 
}

//Function receive UDP message
void receiveMessage()
{
        sockaddr tSocketAddr;
        socklen_t tRxPacketLength;  
        //Open UDP socket
        
        ulSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
           
        // The family is AF_INET
        tSocketAddr.sa_family = AF_INET;  
        
        tSocketAddr.sa_data[0] = 0x11;
        tSocketAddr.sa_data[1] = 0x5C; 
 
        tSocketAddr.sa_data[2] = 0xC0;
        tSocketAddr.sa_data[3] = 0xA8;
        tSocketAddr.sa_data[4] = 0x01;
        tSocketAddr.sa_data[5] = 0x80;
        memset (&tSocketAddr.sa_data[2], 0, 4);  
        bind(ulSocket, &tSocketAddr, sizeof(sockaddr));   
        
        recvfrom(ulSocket, pucCC3000_Rx_Buffer, CC3000_APP_BUFFER_SIZE, 0, &tSocketAddr, &tRxPacketLength); 
}

void checkMessage()
{           
  switch (((char)pucCC3000_Rx_Buffer[0])) {
    
        //START CHARGE
        case 0x30:               
          startCharge();     
          flag_EnableCharge=1;        
          
          break;
          
        //STOP CHARGE    
        case 0x31:    
          stopCharge();
          flag_EnableCharge=0;
          
          break;   
          
        //SEND POWER MEASUREMENT   
        case 0x32:    
          sendPowerMeas();
          closeSocket();
          flag_SendData=0;
          break;  
          
        //CAR STATUS   
        case 0x33:            
          switch (status) {
            //CAR NOT CONNECTED
          case 0:  
            datos="0:NOT";
            break;
            //CAR CONNECTED 
          case 1:  
            datos="0:YES";
            break;      
            //CAR READY TO CAHRGE
          case 2:   
            datos="0:READY";
            break; 
          default:
            break;
          } 
          flag_SendData = 1;
          
          break; 
          
        //CHANGE PWM   
        case 0x34:  
     
          /*DT <8%  		        carga no permitida
          10% < DT <  85%		(%DT) x 0.6 A
          85% < DT <  96%		Corriente disponible = (%DT-64) x 2.5 A
          96% < DT <  97% 	        80A
          97% > 			carga no permitida*/
          
          switch (((char)pucCC3000_Rx_Buffer[1])) {     
          case 0x31:   
          TA0CCR1 = 8000*(0.1);                                 
          break;
          case 0x32:   
          TA0CCR1 = 8000*(0.2);                                  
          break;
          case 0x33:   
          TA0CCR1 = 8000*(0.3);                                   
          break;
          case 0x34:   
          TA0CCR1 = 8000*(0.4);                                
          break;
          case 0x35:   
          TA0CCR1 = 8000*(0.5);                                      
          break;
          case 0x36:   
          TA0CCR1 = 8000*(0.6);                                     
          break;
          case 0x37:   
          TA0CCR1 = 8000*(0.7);                                 
          break;
          case 0x38:   
          TA0CCR1 = 8000*(0.8);                                   
          break;
          case 0x39:   
          TA0CCR1 = 8000*(0.9);                                      
          break;
          case 0x30:   
          TA0CCR1 = 8000*(1);                                      
          break;
            default:
            break;
          }
          
          /*switch (((char)pucCC3000_Rx_Buffer[2])) {  
          
          case 0x31:   
          TA0CCR1 = TA0CCR1 + (8000*(0.01));                                 
          break;
          case 0x32:   
          TA0CCR1 = TA0CCR1 + (8000*(0.02));                                  
          break;
          case 0x33:   
          TA0CCR1 = TA0CCR1 + (8000*(0.03));                                   
          break;
          case 0x34:   
          TA0CCR1 = TA0CCR1 + (8000*(0.04));                                
          break;
          case 0x35:   
          TA0CCR1 = TA0CCR1 + (8000*(0.05));                                      
          break;
          case 0x36:   
          TA0CCR1 = TA0CCR1 + (8000*(0.06));                                     
          break;
          case 0x37:   
          TA0CCR1 = TA0CCR1 + (8000*(0.07));                                 
          break;
          case 0x38:   
          TA0CCR1 = TA0CCR1 + (8000*(0.08));                                   
          break;
          case 0x39:   
          TA0CCR1 = TA0CCR1 + (8000*(0.09));                                      
          break;
            default:
            break;
          }*/
          
          break;
        //RESTART CONSUMPTION MEAS  
        case 0x35:  
          powerSummation=0;      
          
          break;
  default:
    break;
  }
}

//!  @brief  The function triggers a smart configuration process on CC3000.
//!			it exists upon completion of the process

void StartSmartConfig(void)
{
    stopCharge();
    ulSmartConfigFinished = 0;
    ulCC3000Connected = 0;
    ulCC3000DHCP = 0;
    OkToDoShutDown=0;
    
    // Reset all the previous configuration
    wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE);	
    wlan_ioctl_del_profile(255);
    
     // Reset all the previous configuration
    wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE);	
    wlan_ioctl_del_profile(255);
    
    // Wait until CC3000 is disconnected
    while (ulCC3000Connected == 1)
    {
            __delay_cycles(333);
    }
    
    // Trigger the Smart Config process
 	
    wlan_smart_config_set_prefix((char*)aucCC3000_prefix);
	     
    
    // Start the SmartConfig start process
#ifdef CC3000_UNENCRYPTED_SMART_CONFIG
    wlan_smart_config_start(0);
#else
    wlan_smart_config_start(1);
#endif  
    startCharge();                                                                         
    // Wait for Smartconfig process complete
    while (ulSmartConfigFinished == 0)
    {
        __delay_cycles(2000000);
        __delay_cycles(2000000);                     
    }
    
    stopCharge();

#ifndef CC3000_UNENCRYPTED_SMART_CONFIG
    // Create new entry for AES encryption key
    nvmem_create_entry(NVMEM_AES128_KEY_FILEID,16);
    
    // Write AES key to NVMEM
    aes_write_key((unsigned char *)(&smartconfigkey[0]));
    turnLedOn();
    // Decrypt configuration information and add profile
    wlan_smart_config_process();
    turnLedOff();
#endif    
    
    // Configure to connect automatically to the AP retrieved in the 
    // Smart config process
    wlan_ioctl_set_connection_policy(DISABLE, DISABLE, ENABLE);
    
    // Reset the CC3000
    wlan_stop();
    
    __delay_cycles(2000000);
    
    wlan_start(0);
    
    // Mask out all non-required events
    wlan_set_event_mask(HCI_EVENT_MASK);
    
    __delay_cycles(20000000);  
    
    datos= "ConfigOK";            
    sendMessageSmartConfig(datos);    
}

void takeVoltageCurrentPilotSamplesADC()
{  
      //IE1 &= ~(WDTIE);
      __bis_SR_register(GIE);
      
      SD24CCTL2 |= SD24SC;   
}

void checkPilot(unsigned short voltage)
{    
      if(voltage > 65000)
      {
          SmartConfigControl=SmartConfigControl+1;
          if(SmartConfigControl==10)
          { 
              flag_IPConfiguration=1;
          }
      }
      if(voltage< 65000)
      {
          SmartConfigControl=0;
      }
      if(voltage < 25000)
      {
        if(status== 1)
        {
          status = 0;
          datos="0:NOT";
          flag_SendData = 1;
        } 
        if(status== 2)
        {
          status = 0;
          datos="0:NOT";
          flag_SendData = 1;
        } 

      } 
      if(voltage < 19000)
      {
        if(voltage > 15000)
        {
          if(status== 0)
          {       
            status = 1;
            datos="0:YES";
            flag_SendData = 1;
          }  
          if(status== 2)
          {       
            status = 1;
            datos="0:YES";
            flag_SendData = 1;
          }
        }
      } 
      if(voltage < 16200)
      {
        if(voltage > 14000)
        {  
          if(status== 0)
          { 
            status = 2;
            datos="0:REA";
            flag_SendData = 1; 
          }
          if(status== 1)
          { 
            status = 2;
            datos="0:REA";
            flag_SendData = 1; 
          }
        }
      }
}

char statusPilot()
{
    if(datos=="0:NOT")
    {         
          if(vehicleStatusChange==2)
          {
            vehicleStatusChange=0;
            return 1;
          }
          vehicleStatusChange=2;
    }
    
    if(datos=="0:YES")
    {
          if(vehicleStatusChange==3)
          {
            vehicleStatusChange=0;
            return 1;
          }
          vehicleStatusChange=3;
    }
    
    if(datos=="0:REA")
    {
          if(vehicleStatusChange==4)
          {
            vehicleStatusChange=0;
            return 1;
          }
          vehicleStatusChange=4;
    }
    return 0;
     
}
void calculatePower()
{    
      signed long longVoltage,longCurrent,power; 
      
      unsigned short voltage,current;
      
      voltage = SD24MEM0;         // Save CH0 results (clears IFG)
      current = SD24MEM1;         // Save CH1 results (clears IFG)
      
      if(voltage<32768)
      { 
        longVoltage=voltage;
      }
      else
      {
        longVoltage=0xFFFF0000 | ((signed long) voltage);
      }

      if(current<32768)
      {
        longCurrent=current;
      }
      else
      {
        longCurrent=0xFFFF0000 | ((signed long) current);
      }
      power=longVoltage*longCurrent;
      powerSummation = powerSummation + power;   
      //Irms^2= ([sumatorio((Di)^2)]*[FactorEscala^2]) / (Número de muestras)
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
void main(void) {
    init_system();
    /* loop forever */
    while (1) {
        output_logic();
        state_logic();
    }
}

char state_logic(void) {
    char error = 0; 
    switch (present_state) {
    //******************************************IDLE STATE    
    case Idle:            
      if (flag_EnableCharge == 1) {                 
        present_state = Charging;                       
      }      
      if(flag_EnableCharge==0)
      {     
        present_state = Idle; 
      }
      if (flag_SendData == 1)
      {
        present_state = SendData;
      }
      if(flag_IPConfiguration==1)
      {
        present_state = IPConfig;
      }
      break;            
    //***************************************CHARGING STATE        
    case Charging:         
      if(flag_EnableCharge==0)
      {
        present_state = Idle;
      }
      if(flag_EnableCharge==1)
      {
        present_state = Charging;       
      }
      if (flag_SendData == 1) {                 
        present_state = SendData;                 
      } 

      break;        
     //*************************************SENDDATA STATE                 
    case SendData:    
      if ((flag_SendData == 0)&&(flag_EnableCharge==0))
      {
        present_state = Idle;
      }
      if ((flag_SendData == 0)&&(flag_EnableCharge==1))
      {
        present_state = Charging;
      }
      if (flag_SendData == 1)
      {
        present_state = SendData;
      }
                  
      break;
      //************************************IP CONFIG STATE
    case IPConfig:      
      if(flag_IPConfiguration==1)
      {
        present_state = IPConfig;
      }      
      if(flag_IPConfiguration==0)
      {
        present_state = Idle;
      }
      break;
        default:
            error = 1;
    }
    return (error);
}


char output_logic(void) {
    char error = 0;
    switch (present_state) {
    //**************************************   IDLE STATE 
    case Idle: 
        receiveMessage();
        closeSocket();
        checkMessage();                 
      
      break;
    //**************************************   SENDING STATE     
    case Charging:                 
        receiveMessage();  
        closeSocket();
        checkMessage();  

        break;
    
    //**************************************   LISTENING STATE
    case SendData:          
        char pilotState;
        pilotState = statusPilot();
        if(pilotState==1)
        {
          sendMessage(datos);
          closeSocket();
        }
        flag_SendData=0;
      
      break;
    //**************************************  IP CONFIG STATE
    case IPConfig:    
        StartSmartConfig();
        __delay_cycles(20000000); 
        flag_IPConfiguration=0;
      
      break;
    default:            
      error = 1;
    }
    return (error);
}

void init_system(void) {

    WDTCTL = WDTPW + WDTHOLD;
    
    ulCC3000DHCP = 0;
    ulCC3000Connected = 0;
    ulSocket = 0;
    ulSmartConfigFinished=0;

    // Board Initialization start
    initDriver();  
    
    __delay_cycles(20000000); 
    
    stopCharge();
}

#pragma vector=SD24_VECTOR
__interrupt void SD24AISR(void)
{
  unsigned short voltage;
  
  switch (SD24IV)
  {
  case 2:                       // SD24MEM Overflow
    break;
  case 4:                       // SD24MEM0 IFG
    break;
  case 6:                       // SD24MEM1 IFG                
    break;
    
  case 8:                       // SD24MEM2 IFG         
    if(flag_EnableCharge==1)
    {       
        calculatePower(); 
    }
    
    voltage = SD24MEM2;         // Save CH2 results (clears IFG)
    checkPilot(voltage);
    //IE1 |= WDTIE;   
    break;
  }
}

#pragma vector=WDT_VECTOR
__interrupt void WatchDogTimer (void)
{  
    if(controlMeas==3)
    {
       takeVoltageCurrentPilotSamplesADC();
       controlMeas=-1;
    }
    controlMeas=controlMeas+1;
}

