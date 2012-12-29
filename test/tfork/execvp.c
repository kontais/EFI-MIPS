#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define MAX_ARRAY_SIZE 10

int main()
{
  int   WaitStatus;
  int   Index;
  char *ToolArgumentsArray[MAX_ARRAY_SIZE];
  char *ToolName = "ls";


  pid_t pid;


  for (Index = 0; Index< MAX_ARRAY_SIZE; Index++) {
    ToolArgumentsArray[Index] = NULL;
  }
  ToolArgumentsArray[0] = ToolName;
  ToolArgumentsArray[1] = "-l";







  pid = fork();

  if (pid == -1) {
    printf("fork error\n");
  }
  if (pid == 0) {
    return execvp(ToolName,ToolArgumentsArray);
  }
  else
  {
    if(pid == wait(&WaitStatus))
    {
      if (WaitStatus)
        printf("Child Execute Fail\n");
    }
  }

  return 0;
}
