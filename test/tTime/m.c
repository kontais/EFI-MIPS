#include <stdio.h>
#include "EfiCommon.h"
#include "Inode.h"

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define LEAPS_THRU_END_OF(y)    ((y) / 4 - (y) / 100 + (y) / 400)

static const INT32 mon_lengths[2][MONSPERYEAR] = {
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};
static const INT32 year_lengths[2] = {
    DAYSPERNYEAR, DAYSPERLYEAR
};


int loop;
int lastyear = 1970;

void Ext2DecodeTime( unsigned int DayIn)
{
  INT32   Year;
  INT32   Month;
  INT32   Day;

  INT32   NewYear;
  INT32   LeapYear;

  Day     = DayIn;

  Year = EPOCH_YEAR;
  while (Day < 0 || Day >= year_lengths[LeapYear = isleap(Year)]) {
    NewYear = Year + Day / DAYSPERNYEAR;
    if (Day < 0) {
      --NewYear;
    }
    Day -= (NewYear - Year) * DAYSPERNYEAR + 
            LEAPS_THRU_END_OF(NewYear - 1) - 
            LEAPS_THRU_END_OF(Year - 1);
    Year = NewYear;
  }

  for (Month = 0; Day >= mon_lengths[LeapYear][Month]; ++Month) {
    Day = Day - mon_lengths[LeapYear][Month];
  }
  
  Month++;
  Day = Day + 1;

    
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
