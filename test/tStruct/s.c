#include <stdio.h>
#include <stdlib.h>

typedef
void
(*LinuxGetLocalTime) (
  int *Time
);

typedef struct _EFI_LINUX_THUNK_PROTOCOL {
  int                    t;
  LinuxGetLocalTime      GetLocalTime;
} EFI_LINUX_THUNK_PROTOCOL;

void
GetLocalTime(int *Time)
{
  return;
}

EFI_LINUX_THUNK_PROTOCOL mLinuxThunkTable = {
  10,
  GetLocalTime,
};

EFI_LINUX_THUNK_PROTOCOL *gLinux = &mLinuxThunkTable;


int main()
{
  printf("&mLinuxThunkTable=0x%x\n",&mLinuxThunkTable);
  printf("gLinux=0x%x\n",gLinux);
  printf("GetLocalTime=0x%x\n",GetLocalTime);
  printf("&GetLocalTime=0x%x\n",&GetLocalTime);
  printf("gLinux->GetLocalTime=0x%x\n",gLinux->GetLocalTime);
  printf("&gLinux->GetLocalTime=0x%x\n",&gLinux->GetLocalTime);
  return 0;
}

