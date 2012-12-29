#include <stdio.h>

typedef struct _xx {
  char a;
  short b;
  char c;
  int d;
} XX;

typedef union _xxu {
  XX s;
  int i[30];
} XXU;

int main()
{
  int i;
  XXU st;

  for (i = 0; i< 30; i++) {
    st.i[i] = 0;
  }
  st.s.a = '1';
  st.s.b = 0xabcd;
  st.s.c = '2';
  st.s.d = 0x12345678;

  printf("sizeof st = %d\n",sizeof(st));
  return 0;
}
