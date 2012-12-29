int i = 0;
typedef int (*EP)(void);


int EntryPoint(void)
{
  if (i == 8) {
    return 1;
  } else {
    return 2;
  }

  return 0;
}

EP e = EntryPoint;
