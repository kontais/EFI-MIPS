#include "stdio.h"
#include "Frame.h"

extern int SecMain(void);
extern int k_intr(void);

int Status;

int main()
{
  int ReturnValue;

  Status = 0;

  //ReturnValue = SecMain();
  ReturnValue = k_intr();

  printf("ReturnValue=%d\n", ReturnValue);
  return ReturnValue;
}
void interrupt(struct trap_frame *trapframe)
{
  printf("trapframe=0x%x\n", (int)trapframe);
  printf("a0=%d\n", trapframe->a0);
  printf("a1=%d\n", trapframe->a1);
  printf("a2=%d\n", trapframe->a2);
  printf("a3=%d\n", trapframe->a3);
  printf("v0=%d\n", trapframe->v0);
  printf("v1=%d\n", trapframe->v1);
}

int  cpu_info_primary;
