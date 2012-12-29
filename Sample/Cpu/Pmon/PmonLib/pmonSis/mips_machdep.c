#include "types.h"
#include "mips_machdep.h"

static UINTN clkperusec = 398; // = 797000000 /2000000


void
delay(int microseconds)
{
	int total, start;
	start = CPU_GetCOUNT();
	total = microseconds * clkperusec;

	while(total > (CPU_GetCOUNT() - start));
}

void
udelay(int usec)
{
		int total, start;
		start = CPU_GetCOUNT();
		total = usec * clkperusec;

		while(total > (CPU_GetCOUNT() - start))
		{
		};
}
void delay1(int microseconds){
		int total, start;
		start = CPU_GetCOUNT();
		total = microseconds * clkperusec*1000;

		while(total > (CPU_GetCOUNT() - start))
		{
		};
}


