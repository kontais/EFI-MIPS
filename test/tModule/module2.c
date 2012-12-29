#include "moduleall.h"

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

