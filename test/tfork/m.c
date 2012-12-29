#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define MAX_ARRAY_SIZE 10

int main()
{
  int   WaitStatus;
  pid_t pid;
  int WaitSec = 0;
  char *ToolArgumentsArray[MAX_ARRAY_SIZE];
  char *ToolName = "ls";

  memset((void*)ToolArgumentsArray,0x00,MAX_ARRAY_SIZE);
  ToolArgumentsArray[0] = ToolName;
  ToolArgumentsArray[1] = "-l";

  pid = fork();


  if (pid == -1) {
    printf("fork error\n");
  }
  if (pid == 0) {
    printf("I am child\n");
    printf("In c pid = %d\n",pid);
    printf("Child PID = %d\n",getpid());
    return execvp(ToolName,ToolArgumentsArray);
  }
  else
  {
    printf("In p pid = %d\n",pid);
    //while(pid != wait(&WaitStatus))
    if(pid != wait(&WaitStatus))
    {
      //sleep(1);
      printf("Parent Wait %d Second\n",++WaitSec);
    }
    if (WaitStatus)
      printf("Child Execute Fail\n");
  }

  printf("Parent Exit\n");

  return 0;
}
