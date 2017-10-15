void set_Registers(char);
void read_Registers();
char read_Identifier();

//Datos en Flash          //Deben estar en el mismo bloque 0x1FC00
#define Direccion_state 0xC000  //1 xbyte
#define Direccion_duty 0xC001  //2 bytes
#define Direccion_EnergyConsmpt 0xC003  //8 bytes
#define Direccion_Identifier 0xC00B  //8 bytes

