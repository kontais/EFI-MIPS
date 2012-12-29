int  CPU_GetCOUNT(void);
void CPU_SetCOUNT(int);
int  CPU_GetCAUSE(void);
void CPU_SetCAUSE(int);
void  CPU_SetSR(int SetBit, int ClearBit);
int  CPU_GetSTATUS(void);

void delay(int microseconds);
void udelay(int usec);
void delay1(int microseconds);
