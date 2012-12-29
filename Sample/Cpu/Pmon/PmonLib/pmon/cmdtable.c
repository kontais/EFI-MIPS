#include "libc.h"
#include "cmdtable.h"
#include "tgt_machdep.h"
#include "mips_machdep.h"
#include "kbd.h"
#include "cfb_console.h"


extern unsigned int pci_type0(void);
extern unsigned int pci_type1(void);
extern unsigned int misc(void);

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

#define MEMORY_LOG_LEN  (1024*10)

char gMemoryLog[MEMORY_LOG_LEN];
int  gMemoryLogPos = 0;
int  gMemoryLogEnable = 1;
int  MemoryPuts(char *s)
{
	if (gMemoryLogEnable == 0)
		return 0;
		
	while (gMemoryLogPos < MEMORY_LOG_LEN && *s) {
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
extern void* video_fb_address;
extern void CacheFlush(void);


int
cmd_about(void)
{
 int i;	
 int address;
printf("Count   = 0x%x\n",CPU_GetCOUNT());
printf("Status  = 0x%x\n",CPU_GetSTATUS());
printf("Cause   = 0x%x\n",CPU_GetCAUSE());
printf("Config  = 0x%x\n",CPU_GetCONFIG());
printf("allocp1 = 0x%x\n",(int)allocp1);
printf("heaptop = 0x%x\n",(int)heaptop);
printf("video_fb_address = 0x%x\n",(int)video_fb_address);

  address = (int)video_fb_address;
#ifdef xxx
  address -= 0xa0000000;
  address += 0x80000000;
#endif

  /* Blue */
  for (i=0; i< 1024*200; i++) {
    *(unsigned short*) address = 0x001f;
    address += 2;
  }

  /* Green */
  for (i=0; i< 1024*200; i++) {
    *(unsigned short*) address = 0x07e0;
    address += 2;
  }

  /* Red */
  for (i=0; i< 1024*100; i++) {
    *(unsigned short*) address = 0xf800;
    address += 2;
  }
#ifdef xxx
  CacheFlush();
#endif

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

int InitShellExit = 0;

int cmd_exit(void)
{
  InitShellExit = 1;
  
  return( 0 );
}




static const Cmd Cmds[] =
{
  {"about",cmd_about},
  {"exit",cmd_exit},
  {"h",cmd_help},
  {"rb",cmd_reboot},
  {"po",cmd_poweroff},
  {"m",cmd_MemoryLog},
  {"pci0",pci_type0},
  {"pci1",pci_type1},  
  {"misc",misc},  
  {0, 0}
};
			


void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}                  








char            line[LINESZ + 1];   /* input line */


int InitShell(void) {
    while(!InitShellExit) {
        printf("Shell>");
        get_line2(line, 0);
        do_cmd(line);
    }

}



