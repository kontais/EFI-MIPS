#include <stdio.h>



int main()
{
  printf("sizeof(char)      = %d\n",sizeof(char));
  printf("sizeof(short)     = %d\n",sizeof(short));
  printf("sizeof(int)       = %d\n",sizeof(int));
  printf("sizeof(long)      = %d\n",sizeof(long));
  printf("sizeof(long long) = %d\n",sizeof(long long));

  printf("sizeof(char)          = %d\n",sizeof(char));
  printf("sizeof(short int)     = %d\n",sizeof(short int));
  printf("sizeof(int)           = %d\n",sizeof(int));
  printf("sizeof(long int)      = %d\n",sizeof(long int));
  printf("sizeof(long long int) = %d\n",sizeof(long long int));

  return 0;
}
