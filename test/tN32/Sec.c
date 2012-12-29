typedef struct _ss {
  int type;
  int error;
} M2S;

int
module2init ( )
{
  M2S sa;
  M2S sb;
  M2S *psa;
  M2S *psb;
  
  sa.type = 3;
  psa = &sa;
  psb = &sb;

  if (psa->type == 2 || psa->type == 3) {
    psb->type  = psa->type;
  } else {
    psb->error = 1;
    return -1;
  }
  return 0;
}

int gVar0 = 0;
int gVar1;

typedef int (*FUNC)(void);
FUNC m2init = module2init;

int module1init()
{
  gVar0 = 0x1234;
  gVar1 = 0xabcdef;
  module2init();
  return 0;
}

int main()
{
  module1init();
  return 0;
}
