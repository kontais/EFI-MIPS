#include <stdio.h>
#include <stdlib.h>
#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define LEAPS_THRU_END_OF(y)    ((y) / 4 - (y) / 100 + (y) / 400)


static const int year_lengths[2] = {
    365, 366
};


int loop;
int lastyear = 1970;

void Ext2DecodeTime( unsigned int Day)
{
    long        days;
    int         y, newy, yleap;
    

    
    days = Day;

    

    
    y = 1970;
    loop = 0;
    while (days < 0 || days >= (long) year_lengths[yleap = isleap(y)]) {
        newy = y + days / 365;
        if (days < 0)
            --newy;
        days -= (newy - y) * 365 +
            LEAPS_THRU_END_OF(newy - 1) -
            LEAPS_THRU_END_OF(y - 1);
        y = newy;
        loop++;
    }
    
    if (y != lastyear) {
      lastyear = y;
      printf("%d -> %d\n", Day, y);
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
