#include <stdio.h>
#include <limits.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
/*
  printf("Max path is %d\n", PATH_MAX);
  printf("File max is %d\n", NAME_MAX);
*/
  printf("%s\n",getcwd(NULL,0));
  printf("PATH_MAX = %d\n",PATH_MAX);

  return PATH_MAX;
}
