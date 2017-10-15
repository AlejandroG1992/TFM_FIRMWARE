//Functions declaration
void sendMessageSmartConfig(char *datos);
int initDriver(void);
void closeSocket();
void sendMessage(char *datos);
void receiveMessage();

//Extern variables
extern volatile unsigned long ulSmartConfigFinished, ulCC3000Connected,ulCC3000DHCP, OkToDoShutDown, ulCC3000DHCP_configured;
extern volatile unsigned char ucStopSmartConfig;
extern volatile long ulSocket;
extern unsigned char pucCC3000_Rx_Buffer[];
extern const char aucCC3000_prefix[];
//extern static char *datos;

//Parameters
#define CC3000_APP_BUFFER_SIZE                         (2)
#define CC3000_RX_BUFFER_OVERHEAD_SIZE                 (20)   //Antes estaba a 20

#define DISABLE                                        (0)
#define ENABLE                                         (1)

#define SL_VERSION_LENGTH                              (11)
#define NETAPP_IPCONFIG_MAC_OFFSET                     (20)

#define HCI_EVENT_MASK                                 (HCI_EVNT_WLAN_KEEPALIVE | HCI_EVNT_WLAN_UNSOL_INIT | HCI_EVNT_WLAN_ASYNC_PING_REPORT)
//#define HCI_EVENT_MASK                                 (HCI_EVNT_WLAN_UNSOL_INIT | HCI_EVNT_WLAN_ASYNC_PING_REPORT)