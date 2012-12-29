#include "libc.h"
#include "cmdtable.h"
#include "tgt_machdep.h"
#include "mips_machdep.h"
#include "kbd.h"
#include "cfb_console.h"


const Cmd *CmdList[100];

void
cmdlist_expand(	const Cmd *newcmdlist,	int atend)
{
	int i;

	for(i = 0; i < 100; i++) {
		if(CmdList[i] == 0) {
			CmdList[i] = newcmdlist;
			return;
		}
	}
}



int 
do_cmd(char *p)
{
	int	i;
	int clistno;
	const Cmd *CmdTable;


		clistno = 0;
		while((CmdTable = CmdList[clistno++]) != 0) {
			for (i = 0; CmdTable[i].name != 0; i++) {
				if (strcmp (CmdTable[i].name, p) == 0) {
					return (CmdTable[i].func) ();
				}
			}
		}
		printf("command not find, type 'h' for help\n");
	return 0;
}


/**********************************
 *
 * Memory log function start
 *
 *********************************/

#define MEMORY_LOG_LEN  (1024)

char gMemoryLog[MEMORY_LOG_LEN];
int  gMemoryLogPos = 0;
int  gMemoryLogEnable = 1;
int  MemoryPuts(char *s)
{
	if (gMemoryLogEnable == 0)
		return 0;
		
	while (*s && gMemoryLogPos < MEMORY_LOG_LEN) {
		gMemoryLog[gMemoryLogPos++] = *s++;
	}
	return 0;
}


static int gReadPos = 0;
int PrintMemoryLogLine(void)
{
	if (gReadPos > gMemoryLogPos) return 0;
	while(gReadPos < gMemoryLogPos && gMemoryLog[gReadPos] != '\n') {
		printf("%c",gMemoryLog[gReadPos]);
		gReadPos++;
	}
	gReadPos++;
	printf("\n");
	return 1;
}


int
cmd_MemoryLog(void)
{
	char c;

	gMemoryLogEnable = 0;
	gReadPos = 0;
	while(1) {
		c = GetChar();
		if (c != 'q') {
			if(PrintMemoryLogLine() == 0) 
				break;
		}
		else
		{
			break;
		}
	}
	return( 0 );
 }


/**********************************
 *
 * Memory log function start
 *
 *********************************/











extern char *allocp1;
extern char *heaptop;


int
cmd_about(void)
{
	
printf("Count   = 0x%x\n",CPU_GetCOUNT());
printf("Status  = 0x%x\n",CPU_GetSTATUS());
printf("Cause   = 0x%x\n",CPU_GetCAUSE());
printf("allocp1 = 0x%x\n",(int)allocp1);
printf("heaptop = 0x%x\n",(int)heaptop);



 return( 0 );
 }




int
cmd_help(void)
{
	int i,j;
	const Cmd *CmdTable;

	for(j =0; j < 100; j++) {
		CmdTable = CmdList[j];
		if (CmdTable) {
			for (i = 0; CmdTable[i].name != 0; i++) {
				printf("%s\n",CmdTable[i].name);
			}
			continue;
		}
		else
			break;
	}

	return (0);
}


int cmd_poweroff(void)
{
    tgt_poweroff();
    return 0;
}
	

int
cmd_reboot (void)
{
    tgt_reboot();
    return(0);
}






static const Cmd Cmds[] =
{
	{"about",cmd_about},
	{"h",cmd_help},
	{"rb",cmd_reboot},
    {"po",cmd_poweroff},
    {"m",cmd_MemoryLog},
	{0, 0}
};
			


void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}                  








char            line[LINESZ + 1];   /* input line */


int InitShell(void) {
    while(1) {
        printf("Shell>");
        get_line2(line, 0);
        do_cmd(line);
    }

}



