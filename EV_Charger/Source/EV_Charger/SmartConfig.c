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
#include "security.h"
#include "Main.h"
#include "board.h"
#include "Analog_sampling.h"
#include "ControlPilot.h"
#include "Listener.h"
#include "SmartConfig.h"
#include "Wattmeter.h"
#include "Wifi.h"

#include <msp430.h>


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
    wlan_ioctl_set_connection_policy(DISABLE, ENABLE, ENABLE);
    
    // Reset the CC3000
    wlan_stop();
    
    __delay_cycles(2000000);
    
    wlan_start(0);
    
    // Mask out all non-required events
    wlan_set_event_mask(HCI_EVENT_MASK);
    
    __delay_cycles(20000000);  
    
    //datos= "ConfigOK";            
        
}