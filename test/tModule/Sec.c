#include "moduleall.h"
#include "Tiano.h"
#include "SystemLib.h"
#include "stdio.h"
#include "stdarg.h"

int
SystemLib_printf (
  const char * Format,
  ...
  )
{
  int Return;
  va_list Marker;

  va_start (Marker, Format);
  Return = vprintf (Format,Marker);
  va_end (Marker);

  return Return;
}


EFI_SYSTEM_LIB_PROTOCOL mSystemLib = {
  0,
  SystemLib_printf,
  0
};


EFI_SYSTEM_LIB_PROTOCOL *gSys = &mSystemLib;



unsigned int gPpi[1];

int main()
{
  int Status;
  gPpi[0] = (UINT32) gSys;

  Status = moduleEntry(gPpi);
  return Status;
}
