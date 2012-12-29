#ifndef _CMDTABLE_
#define _CMDTABLE_

typedef struct _Cmd {
	const char    *name;
	int	         (*func) (void);
} Cmd;




int do_cmd(char*p);
int  MemoryPuts(char *s);
int PrintMemoryLogLine(void);
void init_cmd(void);
int InitShell(void);


#endif // _CMDTABLE_