typedef unsigned int UINTN;


typedef double  ALIGN;


union header {
  struct {
    union header   *ptr;
    UINTN        size;
  } s;
  ALIGN           x;
};

typedef union header HEADER;

static HEADER   base;

void tm(void)
{
  base.s.ptr =  &base;
  base.s.size = 0;
}

