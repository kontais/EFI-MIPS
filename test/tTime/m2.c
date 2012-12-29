#include <stdio.h>
#include "EfiCommon.h"
#include "Inode.h"

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

#define PASS_LEAP_YEARS(y)    ((y) / 4 - (y) / 100 + (y) / 400)

static const UINT32 mon_lengths[2][MONSPERYEAR] = {
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};
static const UINT32 year_lengths[2] = {
    DAYSPERNYEAR, DAYSPERLYEAR
};


int loop;
int lastyear = 1970;

UINT32 GetPassDays(UINT32 Year)
{
  UINT32 PassYear = Year - 1;
  return PASS_LEAP_YEARS(PassYear) * 366 + (PassYear - PASS_LEAP_YEARS(PassYear)) * 365;
}

void Ext2DecodeTime( unsigned int DayIn)
{
  UINT32   Year;
  //UINT32   Month;
  UINT32   Day;

  UINT32   PassDays;

  Day     = DayIn;

  Year = EPOCH_YEAR;
  
  PassDays = GetPassDays(1970);
  Year     = (PassDays + DayIn) / 365;
  
  while (GetPassDays(Year) > (PassDays + DayIn)) {
    Year--;
  };
  


    
    if (Year != lastyear) {
      lastyear = Year;
      printf("%d -> %d\n", DayIn, Year);
    }
}


int main()
{
  int index = 1;
  
  for (index = 1; index < (2020 - 1970) * 366; index ++) {
    Ext2DecodeTime(index);
}  

  printf("loop %d\n", loop);
  return 0;
}
