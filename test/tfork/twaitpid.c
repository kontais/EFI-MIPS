#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

int main()
{
  pid_t child_pid, wpid;
  int status;

  child_pid = fork();

  if (child_pid == -1)
  {
    printf("fork error\n");
    exit(EXIT_FAILURE);
  }
  if (child_pid == 0)
  {
    printf("in c child_pid = %d\n",child_pid);
    printf("c id = %d\n",getpid());
    printf("p id = %d\n",getppid());
    sleep(3);
   // return 3;
   exit(4);
  }
  else
  {
    printf("p2 id = %d\n",getpid());
    do {
      printf("child_pid = %d\n",child_pid);
      wpid = waitpid(child_pid, &status,WUNTRACED|WCONTINUED);
      if (wpid == -1)
      {
        printf("waitpid error\n");
        exit(EXIT_FAILURE);
      }

      if (WIFEXITED(status)) {
        printf("child exited, status = %d\n",WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        printf("child killed (signal %d) \n",WTERMSIG(status));
      } else if (WIFSTOPPED(status)) {
        printf("child stopped (signal %d) \n",WSTOPSIG(status));
      } else if (WIFCONTINUED(status)) {
        printf("child continued\n");
      } else {
        printf("Unexpected status (0x%x) \n",status);
      }
    }while(!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 0;
}
