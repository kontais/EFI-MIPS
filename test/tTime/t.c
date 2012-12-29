#include <stdio.h>
#include <stdlib.h>

#define	INT_MAX		0x7fffffff	/* max value for an int */
#define	INT_MIN		(-0x7fffffff-1)	/* min value for an int */

#define FALSE 0
#define TRUE 1



#define isleaptest(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))


static int
leaps_thru_end_of(y)
register const int	y;
{
	return (y >= 0) ? (y / 4 - y / 100 + y / 400) :
		-(leaps_thru_end_of(-(y + 1)) + 1);
}


static const int year_lengths[2] = {
    365, 366
};

static int
increment_overflow(ip, j)
int * const	ip;
int		j;
{
	register int const	i = *ip;

	/*
	** If i >= 0 there can only be overflow if i + j > INT_MAX
	** or if j > INT_MAX - i; given i >= 0, INT_MAX - i cannot overflow.
	** If i < 0 there can only be overflow if i + j < INT_MIN
	** or if j < INT_MIN - i; given i < 0, INT_MIN - i cannot overflow.
	*/
	if ((i >= 0) ? (j > INT_MAX - i) : (j < INT_MIN - i))
		return TRUE;
	*ip += j;
	return FALSE;
}


int loop;
int lastyear = 1970;

int Ext2DecodeTime( unsigned int Day)
{
		register time_t	tdelta;
		register int	idelta;
		register int	leapdays;
		int		newy;    
		int tdays;
		int y;
    

    

	y = 1970;
	tdays = Day;
	loop = 0;
	while (tdays < 0 || tdays >= year_lengths[isleaptest(y)]) {


		tdelta = tdays / 366;
		idelta = tdelta;
		if (tdelta - idelta >= 1 || idelta - tdelta >= 1)
			return 0;
		if (idelta == 0)
			idelta = (tdays < 0) ? -1 : 1;
		newy = y;
		if (increment_overflow(&newy, idelta))
			return 0;
		leapdays = leaps_thru_end_of(newy - 1) -
			leaps_thru_end_of(y - 1);
		tdays -= ((time_t) newy - y) * 365;
		tdays -= leapdays;
		y = newy;
		
		loop++;
	}

    if (y != lastyear) {
      lastyear = y;
      printf("%d -> %d\n", Day, y);
    }
    
    return y;
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
