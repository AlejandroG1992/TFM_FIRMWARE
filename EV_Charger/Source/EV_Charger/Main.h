//External variables

extern signed long  long powerSummation;                //64 bits
extern char Pilot_State; 
extern unsigned short Pilot_Num;
extern char Charger_State;
extern char WDT_counter;
//extern char prue;


//Define Pilot States
#define A 0   //Car disconnected
#define A2 1   //Car disconnected and power available
#define B1 2   //Car connected but not power available
#define B2 3   //Car connected and power available
#define C1 4   //Car ready to charge but not power available , no Ventilation is required
#define C2 5   //Car ready and power available, no Ventilation is required
#define D1 6   //Car ready but not power available , Ventilation is required
#define D2 7   //Car ready and power available, Ventilation is required
#define E  8   //SmartConfig in progress

//Define Car States
#define Not_Connected 0
#define Connected 1
#define Charging 2
#define Error 3
#define SmartConfig_Inic 4
#define SmartConfig_Exec 5
#define SmartConfig_Assign 6